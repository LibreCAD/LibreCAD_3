#include "renderer.h"
#include "shader_p.h"
#include "device_p.h"
#include <QCryptographicHash>
#include <QFile>
#include <QGuiApplication>
#include <QResource>
#include <QVersionNumber>
#include <rhi/qrhi_platform.h>
#include <rhi/qshaderdescription.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <thread>
#if defined(Q_OS_MACOS)
#include <IOKit/IOKitLib.h>
#include <objc/message.h>
#include <objc/runtime.h>
#endif

// Keep the initializer at global scope (Q_INIT_RESOURCE's namespace contract).
static void initializeRhiResourceObject() { Q_INIT_RESOURCE(lc_rhi_shaders); }

namespace lc { namespace rhi {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
using viewer::render::Frame;
using viewer::render::Vertex;
using viewer::render::Snapshot;
static_assert(sizeof(Vertex) == 6 * sizeof(float), "RHI vertex layout must be packed floats");
detail::MetalIdentity selectedMetalIdentity(QRhi* device) {
    detail::MetalIdentity result;
#if defined(Q_OS_MACOS)
    if (device->backend() != QRhi::Metal) return result;
    const auto* handles = static_cast<const QRhiMetalNativeHandles*>(device->nativeHandles());
    if (!handles || !handles->dev) return result;
    const id metal = reinterpret_cast<id>(handles->dev);
    const SEL registry = sel_registerName("registryID");
    if (!class_respondsToSelector(object_getClass(metal), registry)) return result;
    // registryID is a scalar uint64_t on both Metal ABIs; never inspect a different GPU.
    const auto registryId = reinterpret_cast<std::uint64_t (*)(id, SEL)>(objc_msgSend)(metal, registry);
    if (!registryId) return result;
    const auto matching = IORegistryEntryIDMatching(registryId);
    if (!matching) return result;
    const auto entry = IOServiceGetMatchingService(kIOMainPortDefault, matching); // consumes matching
    if (!entry) return result;
    std::uint64_t actualId = 0;
    io_name_t name = {};
    const bool verified = IORegistryEntryGetRegistryEntryID(entry, &actualId) == KERN_SUCCESS && actualId == registryId &&
            IOObjectGetClass(entry, name) == KERN_SUCCESS && IOObjectConformsTo(entry, "AppleParavirtAccelerator");
    IOObjectRelease(entry); // Release the native reference before potentially throwing string allocation.
    if (verified) {
        result.registryId = registryId;
        result.registryClass = name;
    }
#else
    (void)device;
#endif
    return result;
}
}

namespace detail {
void initializeResources() {
    static const bool initialized = [] { initializeRhiResourceObject(); return true; }();
    (void)initialized;
}
QShader checkedShader(const QByteArray& bytes, QShader::Stage stage) {
    const auto s = QShader::fromSerialized(bytes);
    require(s.isValid() && s.stage() == stage, "Missing, corrupt or wrong-stage RHI shader package");
    require(!s.shader(QShaderKey(QShader::SpirvShader, QShaderVersion(100))).shader().isEmpty() &&
            !s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12))).shader().isEmpty(),
            "RHI shader lacks required SPIR-V/MSL variants");
    for (const auto& key : s.availableShaders()) {
        require(key.sourceVariant() == QShader::StandardShader &&
                ((key.source() == QShader::SpirvShader && key.sourceVersion() == QShaderVersion(100)) ||
                 (key.source() == QShader::MslShader && key.sourceVersion() == QShaderVersion(12))),
                "Unexpected RHI shader language/version/variant");
    }
    const auto d = s.description();
    require(d.isValid() && d.combinedImageSamplers().isEmpty() && d.storageBlocks().isEmpty() &&
            d.pushConstantBlocks().isEmpty() && d.storageImages().isEmpty() &&
            d.separateImages().isEmpty() && d.separateSamplers().isEmpty(), "Unexpected RHI shader bindings");
    const auto variables = [](const auto& actual, std::initializer_list<QShaderDescription::VariableType> types) {
        if (actual.size() != int(types.size())) return false;
        int location = 0;
        for (auto type : types) {
            const auto it = std::find_if(actual.cbegin(), actual.cend(), [&](const auto& v) { return v.location == location; });
            if (it == actual.cend() || it->type != type || !it->arrayDims.isEmpty()) return false;
            ++location;
        }
        return true;
    };
    using D = QShaderDescription;
    require(variables(d.outputVariables(), {D::Vec4}), "RHI shader output layout mismatch");
    if (stage == QShader::VertexStage) {
        require(variables(d.inputVariables(), {D::Vec2, D::Vec4}) && d.uniformBlocks().size() == 1,
                "RHI vertex input/uniform layout mismatch");
        const auto u = d.uniformBlocks().first();
        require(u.binding == 0 && u.descriptorSet == 0 && u.size == 64 && u.members.size() == 1 &&
                u.members.first().type == D::Mat4 && u.members.first().offset == 0 &&
                u.members.first().size == 64 && u.members.first().matrixStride == 16 &&
                !u.members.first().matrixIsRowMajor && u.members.first().arrayDims.isEmpty() &&
                u.members.first().structMembers.isEmpty(), "RHI uniform packing mismatch");
    } else {
        require(stage == QShader::FragmentStage && variables(d.inputVariables(), {D::Vec4}) &&
                d.uniformBlocks().isEmpty(), "RHI fragment input layout mismatch");
    }
    return s;
}
QShader resourceShader(const char* resource, QShader::Stage stage) {
    initializeResources();
    QFile file(QString::fromUtf8(resource));
    require(file.open(QIODevice::ReadOnly), "Missing embedded RHI shader resource");
    return checkedShader(file.readAll(), stage);
}
}

namespace {
// One executor for owned and future borrowed hosts. It never begins/ends a frame.
class Executor {
public:
    struct Counts { unsigned draws = 0, indexed = 0; };
    Executor() : vertex(detail::resourceShader(":/librecad/rhi/triangles.vert.qsb", QShader::VertexStage)),
                 fragment(detail::resourceShader(":/librecad/rhi/triangles.frag.qsb", QShader::FragmentStage)) {}
    void releaseTarget() { pipeline.reset(); pass = nullptr; }
    void prepare(QRhi* rhi, QRhiRenderTarget* target, const Frame& f) {
        require(!device || device == rhi, "Executor cannot cross device owners");
        device = rhi;
        const int required = std::max(int(sizeof(Vertex)), int(f.vertices.size() * sizeof(Vertex)));
        if (!vertices || vertices->size() < quint32(required)) {
            vertices.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, required));
            require(vertices && vertices->create(), "RHI vertex allocation failed");
        }
        const int indexBytes = int(f.indices.size() * sizeof(std::uint32_t));
        if (indexBytes && (!indices || indices->size() < quint32(indexBytes))) {
            indices.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::IndexBuffer, indexBytes));
            require(indices && indices->create(), "RHI index allocation failed");
        }
        if (!uniforms) {
            uniforms.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
            require(uniforms->create(), "RHI uniform allocation failed");
            bindings.reset(rhi->newShaderResourceBindings());
            bindings->setBindings({QRhiShaderResourceBinding::uniformBuffer(0,
                QRhiShaderResourceBinding::VertexStage, uniforms.get())});
            require(bindings->create(), "RHI bindings creation failed");
        }
        if (!pipeline || pass != target->renderPassDescriptor()) {
            pipeline.reset(rhi->newGraphicsPipeline());
            pipeline->setShaderStages({{QRhiShaderStage::Vertex, vertex}, {QRhiShaderStage::Fragment, fragment}});
            QRhiVertexInputLayout layout;
            layout.setBindings({{sizeof(Vertex)}});
            layout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                                  {0, 1, QRhiVertexInputAttribute::Float4, 2 * sizeof(float)}});
            pipeline->setVertexInputLayout(layout);
            pipeline->setCullMode(QRhiGraphicsPipeline::None);
            pipeline->setDepthTest(false); pipeline->setDepthWrite(false);
            QRhiGraphicsPipeline::TargetBlend blend;
            blend.enable = true;
            blend.srcColor = blend.srcAlpha = QRhiGraphicsPipeline::One;
            blend.dstColor = blend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
            pipeline->setTargetBlends({blend});
            pipeline->setShaderResourceBindings(bindings.get());
            pass = target->renderPassDescriptor();
            pipeline->setRenderPassDescriptor(pass);
            pipeline->setSampleCount(1);
            require(pipeline->create(), "RHI pipeline creation failed");
        }
    }
    Counts render(QRhiCommandBuffer* cb, QRhiRenderTarget* target, const Frame& f) {
        Counts counts;
        auto* updates = device->nextResourceUpdateBatch();
        QMatrix4x4 matrix = device->clipSpaceCorrMatrix();
        matrix.ortho(0, float(f.width), float(f.height), 0, -1, 1);
        updates->updateDynamicBuffer(uniforms.get(), 0, 64, matrix.constData());
        if (!f.vertices.empty())
            updates->updateDynamicBuffer(vertices.get(), 0, int(f.vertices.size() * sizeof(Vertex)), f.vertices.data());
        if (!f.indices.empty())
            updates->updateDynamicBuffer(indices.get(), 0, int(f.indices.size() * sizeof(std::uint32_t)), f.indices.data());
        cb->beginPass(target, QColor::fromRgbF(f.clear[0], f.clear[1], f.clear[2], f.clear[3]), {1, 0}, updates);
        cb->setGraphicsPipeline(pipeline.get()); cb->setShaderResources(bindings.get());
        cb->setViewport({0, 0, float(f.width), float(f.height)});
        if (!f.vertices.empty()) {
            const QRhiCommandBuffer::VertexInput input(vertices.get(), 0);
            cb->setVertexInput(0, 1, &input, f.indices.empty() ? nullptr : indices.get(), 0,
                               QRhiCommandBuffer::IndexUInt32);
            if (f.indices.empty()) {
                cb->draw(quint32(f.vertices.size()));
            } else {
                cb->drawIndexed(quint32(f.indices.size()));
                ++counts.indexed;
            }
            ++counts.draws;
        }
        cb->endPass();
        return counts;
    }
    QByteArray shaderIdentity() const {
        return QCryptographicHash::hash(vertex.serialized() + fragment.serialized(), QCryptographicHash::Sha256).toHex();
    }
private:
    QShader vertex, fragment;
    QRhi* device = nullptr;
    QRhiRenderPassDescriptor* pass = nullptr;
    std::unique_ptr<QRhiBuffer> vertices, indices, uniforms;
    std::unique_ptr<QRhiShaderResourceBindings> bindings;
    std::unique_ptr<QRhiGraphicsPipeline> pipeline;
};
class OwnedFrame {
public:
    explicit OwnedFrame(QRhi* rhi) : device(rhi) {
        require(device->beginOffscreenFrame(&cb) == QRhi::FrameOpSuccess, "RHI begin frame failed");
    }
    ~OwnedFrame() { if (active) device->endOffscreenFrame(); }
    QRhi::FrameOpResult finish() { active = false; return device->endOffscreenFrame(); }
    QRhiCommandBuffer* cb = nullptr;
private:
    QRhi* device;
    bool active = true;
};
}

struct OffscreenRenderer::Impl {
    // These CPU result/callback owners outlive device/backend destruction, even
    // after a failed submit. Never reuse them after a terminal host failure.
    QRhiReadbackResult readback;
    bool completed = false, failed = false;
#ifndef Q_OS_MACOS
    QVulkanInstance instance;
#endif
    std::unique_ptr<QRhi> device;
    Executor executor;
    std::unique_ptr<QRhiTexture> texture;
    std::unique_ptr<QRhiRenderPassDescriptor> pass;
    std::unique_ptr<QRhiTextureRenderTarget> target;
    const std::thread::id owner = std::this_thread::get_id();
    DeviceInfo info;
    ~Impl() {
        // Pipelines must be released before their target/pass descriptors.
        executor.releaseTarget(); target.reset(); pass.reset(); texture.reset();
    }
    explicit Impl(Api api, int policyLimit) {
        require(qobject_cast<QGuiApplication*>(QCoreApplication::instance()), "RHI requires a live QGuiApplication");
        require(QString::fromLatin1(qVersion()) == QString::fromLatin1(QT_VERSION_STR), "Private RHI Qt build/runtime mismatch");
#ifdef Q_OS_MACOS
        if (api != Api::Metal) throw std::invalid_argument("This RHI provider requires Metal (no fallback)");
        QRhiMetalInitParams params;
        device.reset(QRhi::create(QRhi::Metal, &params));
#else
        if (api != Api::Vulkan) throw std::invalid_argument("This RHI provider requires Vulkan (no fallback)");
        instance.setApiVersion(instance.supportedApiVersion() >= QVersionNumber(1, 1)
                               ? QVersionNumber(1, 1) : QVersionNumber(1, 0));
        instance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
        require(instance.create(), "RHI Vulkan instance creation failed");
        QRhiVulkanInitParams params; params.inst = &instance;
        device.reset(QRhi::create(QRhi::Vulkan, &params));
#endif
        require(bool(device), "Requested RHI device creation failed (no fallback)");
        require(device->backend() == (api == Api::Metal ? QRhi::Metal : QRhi::Vulkan), "RHI backend mismatch");
        require(device->isTextureFormatSupported(QRhiTexture::RGBA8, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource) &&
                device->supportedSampleCounts().contains(1) && device->resourceLimit(QRhi::MaxUniformBufferRange) >= 64,
                "RHI device lacks required RGBA8/readback/uniform capabilities");
        const int minimum = device->resourceLimit(QRhi::TextureSizeMin);
        const int maximum = device->resourceLimit(QRhi::TextureSizeMax);
        require(minimum > 0 && maximum >= minimum, "Unavailable RHI texture capability limits");
        if (policyLimit < minimum) throw std::invalid_argument("Texture policy is below the device minimum");
        const auto native = device->driverInfo();
        const auto identity = selectedMetalIdentity(device.get());
#if defined(Q_OS_MACOS) && defined(Q_PROCESSOR_X86_64)
        const bool intelMetal = api == Api::Metal;
#else
        const bool intelMetal = false;
#endif
        const auto kind = detail::deviceClass(native.deviceType, intelMetal, identity);
        info = {api, QString::fromUtf8(native.deviceName).toStdString(), kind, qVersion(),
                executor.shaderIdentity().toStdString(), minimum, maximum, std::min(policyLimit, maximum),
                int(native.deviceType), identity.registryId ? std::to_string(identity.registryId) : "", identity.registryClass};
    }
    void checkOwner() const {
        if (std::this_thread::get_id() != owner) throw std::logic_error("RHI access requires its owner thread");
    }
    Snapshot render(const Frame& f) {
        checkOwner();
        require(!failed, "RHI host has a latched terminal failure");
        viewer::render::validateFrame(f); // Invalid input cannot mutate host/resources.
        if (f.width == 0 || f.height == 0) {
            executor.releaseTarget(); target.reset(); pass.reset(); texture.reset();
            completed = false; readback = QRhiReadbackResult();
            Snapshot suspended;
            suspended.width = f.width; suspended.height = f.height;
            suspended.targetReady = bool(target);
            return suspended; // No target allocation, pass, submit or accepted pixels.
        }
        if (f.width < info.textureSizeMin || f.height < info.textureSizeMin ||
            f.width > info.textureLimit || f.height > info.textureLimit)
            throw TextureSizeLimitError(info.textureLimit);
        try {
            if (!texture || texture->pixelSize() != QSize(f.width, f.height)) {
                executor.releaseTarget(); target.reset(); pass.reset(); texture.reset();
                texture.reset(device->newTexture(QRhiTexture::RGBA8, {f.width, f.height}, 1,
                    QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
                require(texture->create(), "RHI target texture allocation failed");
                target.reset(device->newTextureRenderTarget({texture.get()}));
                pass.reset(target->newCompatibleRenderPassDescriptor());
                target->setRenderPassDescriptor(pass.get());
                require(target->create(), "RHI render target creation failed");
            }
            executor.prepare(device.get(), target.get(), f);
            completed = false; readback = QRhiReadbackResult();
            readback.completed = [this] { completed = true; };
            OwnedFrame frame(device.get());
            const auto counts = executor.render(frame.cb, target.get(), f);
            auto* updates = device->nextResourceUpdateBatch();
            updates->readBackTexture(QRhiReadbackDescription(texture.get()), &readback);
            frame.cb->resourceUpdate(updates);
            require(frame.finish() == QRhi::FrameOpSuccess, "RHI submit failed");
            require(completed && readback.format == QRhiTexture::RGBA8 && readback.pixelSize == QSize(f.width, f.height) &&
                    readback.data.size() == qsizetype(f.width) * f.height * 4, "Incomplete or invalid RHI readback");
            Snapshot result{f.width, f.height, std::vector<std::uint8_t>(std::size_t(f.width) * f.height * 4)};
            result.drawCount = counts.draws; result.indexedDrawCount = counts.indexed;
            result.targetReady = bool(target);
            const std::size_t stride = std::size_t(f.width) * 4;
            for (int y = 0; y < f.height; ++y) {
                const int source = device->isYUpInFramebuffer() ? f.height - 1 - y : y;
                std::memcpy(result.rgba.data() + y * stride, readback.data.constData() + source * stride, stride);
            }
            return result;
        } catch (...) { failed = true; throw; }
    }
};

OffscreenRenderer::OffscreenRenderer(Api api, int textureLimit) : impl(new Impl(api, textureLimit)) {}
OffscreenRenderer::~OffscreenRenderer() = default;
Snapshot OffscreenRenderer::render(const Frame& f) { return impl->render(f); }
DeviceInfo OffscreenRenderer::deviceInfo() const { impl->checkOwner(); return impl->info; }
std::unique_ptr<OffscreenRenderer> createOffscreenRenderer(Api api, int textureLimit) {
    if (textureLimit < 1 || textureLimit > viewer::render::MaxFrameDimension)
        throw std::invalid_argument("Texture policy must be within the CPU resource bound");
    return std::unique_ptr<OffscreenRenderer>(new OffscreenRenderer(api, textureLimit));
}
}}
