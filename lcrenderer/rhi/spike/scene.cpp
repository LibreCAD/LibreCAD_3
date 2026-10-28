#include "scene.h"
#include "support.h"
#include <QFile>
#include <QMatrix4x4>
#include <QPointF>
#include <rhi/qshader.h>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool ok, const char *message) {
    if (!ok) throw std::runtime_error(message);
}
QShader shader(const char *path) {
    QFile file(QString::fromLatin1(path));
    require(file.open(QIODevice::ReadOnly), "Embedded shader missing");
    QShader result = QShader::fromSerialized(file.readAll());
    require(result.isValid(), "Invalid shader package");
    return result;
}
}

void SpikeScene::quad(float x, float y, float w, float h, QColor c, QRectF uv) {
    const float a = c.alphaF();
    auto vertex = [&](float px, float py, float u, float v) {
        return Vertex{px, py, float(c.redF()) * a, float(c.greenF()) * a,
                      float(c.blueF()) * a, a, u, v};
    };
    const Vertex tl = vertex(x, y, uv.left(), uv.top());
    const Vertex tr = vertex(x + w, y, uv.right(), uv.top());
    const Vertex bl = vertex(x, y + h, uv.left(), uv.bottom());
    const Vertex br = vertex(x + w, y + h, uv.right(), uv.bottom());
    vertices.insert(vertices.end(), {tl, bl, tr, tr, bl, br});
}

void SpikeScene::buildFixture() {
    // Original tiny bitmap/image fixtures, distributed under the project LICENSE.
    // This exercises a GPU atlas, not production text shaping or font fallback.
    atlas = QImage(32, 16, QImage::Format_RGBA8888_Premultiplied);
    atlas.fill(Qt::transparent);
    atlas.setPixelColor(0, 0, Qt::white);
    const char *glyphs[] = {
        "01110100001000010000100001000001110", // C, 5 x 7
        "01110100011000111111100011000110001", // A
        "11110100011000110001100011000111110"  // D
    };
    for (int g = 0; g < 3; ++g) {
        for (int y = 0; y < 7; ++y)
            for (int x = 0; x < 5; ++x)
                if (glyphs[g][y * 5 + x] == '1')
                    atlas.setPixelColor(1 + g * 6 + x, 1 + y, Qt::white);
    }
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            atlas.setPixelColor(20 + x, y,
                y < 4 ? (x < 4 ? Qt::red : Qt::green) : (x < 4 ? Qt::blue : Qt::yellow));
    atlas.setPixelColor(26, 6, QColor(0, 0, 255, 128));

    quad(12, 12, 36, 24, Qt::red);
    // Two joined strokes as triangle strips with a miter at the common endpoint.
    const QPointF points[] = {{64, 20}, {88, 20}, {88, 44}};
    const QPointF offsets[] = {{0, 3}, {-3, 3}, {-3, 0}};
    auto cyan = [](QPointF p) { return Vertex{float(p.x()), float(p.y()), 0, 1, 1, 1,
                                            0.5f / 32, 0.5f / 16}; };
    for (int i = 0; i < 2; ++i) {
        auto a = cyan(points[i] + offsets[i]), b = cyan(points[i] - offsets[i]);
        auto c = cyan(points[i + 1] + offsets[i + 1]), d = cyan(points[i + 1] - offsets[i + 1]);
        vertices.insert(vertices.end(), {a, b, c, c, b, d});
    }
    for (int x = 16; x < 112; x += 16) quad(x, 58, 8, 4, Qt::yellow);
    quad(128, 12, 32, 32, Qt::green);
    quad(144, 28, 32, 28, QColor(255, 0, 0, 128));
    quad(192, 24, 32, 32, QColor(255, 0, 0, 128));
    quad(208, 40, 32, 32, QColor(0, 0, 255, 128));
    for (int g = 0; g < 3; ++g)
        quad(16 + g * 18, 92, 15, 21, Qt::white,
             QRectF((1.0 + g * 6) / 32, 1.0 / 16, 5.0 / 32, 7.0 / 16));
    quad(112, 96, 64, 64, Qt::white, QRectF(20.0 / 32, 0, 8.0 / 32, 8.0 / 16));
    const double origin = 1e12;
    quad(float((origin + 212) - origin), float((origin + 100) - origin), 24, 24, Qt::magenta);
}

void SpikeScene::render(QRhi *rhi, QRhiRenderTarget *target, QRhiCommandBuffer *cb, bool transparent) {
    require(rhi && target && cb, "Missing frame owner/target");
    requireSpikeCapabilities(rhi, target->pixelSize());
    if (!device) {
        device = rhi;
        buildFixture();
        vertexBuffer.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                         int(vertices.size() * sizeof(Vertex))));
        uniformBuffer.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
        texture.reset(rhi->newTexture(QRhiTexture::RGBA8, atlas.size()));
        sampler.reset(rhi->newSampler(QRhiSampler::Nearest, QRhiSampler::Nearest,
                                     QRhiSampler::None, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        require(vertexBuffer->create() && uniformBuffer->create() && texture->create() && sampler->create(),
                "RHI resource creation failed");
        bindings.reset(rhi->newShaderResourceBindings());
        bindings->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, uniformBuffer.get()),
            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, texture.get(), sampler.get())});
        require(bindings->create(), "RHI bindings creation failed");
    }
    require(device == rhi, "Scene reused across device generations");
    if (!pipeline || pass != target->renderPassDescriptor() || samples != target->sampleCount()) {
        pipeline.reset(rhi->newGraphicsPipeline());
        pipeline->setShaderStages({{QRhiShaderStage::Vertex, shader(":/spike/scene.vert.qsb")},
                                   {QRhiShaderStage::Fragment, shader(":/spike/scene.frag.qsb")}});
        QRhiVertexInputLayout layout;
        layout.setBindings({{sizeof(Vertex)}});
        layout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                              {0, 1, QRhiVertexInputAttribute::Float4, 2 * sizeof(float)},
                              {0, 2, QRhiVertexInputAttribute::Float2, 6 * sizeof(float)}});
        pipeline->setVertexInputLayout(layout);
        QRhiGraphicsPipeline::TargetBlend blend;
        blend.enable = true;
        blend.srcColor = blend.srcAlpha = QRhiGraphicsPipeline::One;
        blend.dstColor = blend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        pipeline->setTargetBlends({blend});
        pipeline->setShaderResourceBindings(bindings.get());
        pass = target->renderPassDescriptor();
        samples = target->sampleCount();
        pipeline->setRenderPassDescriptor(pass);
        pipeline->setSampleCount(samples);
        require(pipeline->create(), "RHI graphics pipeline creation failed");
    }
    auto *updates = rhi->nextResourceUpdateBatch();
    if (!uploadBytes) {
        updates->uploadStaticBuffer(vertexBuffer.get(), vertices.data());
        updates->uploadTexture(texture.get(), atlas);
        uploadBytes = qint64(vertices.size() * sizeof(Vertex)) + atlas.sizeInBytes();
    }
    QMatrix4x4 matrix = rhi->clipSpaceCorrMatrix();
    matrix.ortho(0, 256, 192, 0, -1, 1);
    updates->updateDynamicBuffer(uniformBuffer.get(), 0, 64, matrix.constData());
    cb->beginPass(target, transparent ? QColor(0, 0, 0, 0) : QColor(16, 24, 32), {1.0f, 0}, updates);
    cb->setGraphicsPipeline(pipeline.get());
    cb->setShaderResources(bindings.get());
    const QSize size = target->pixelSize();
    cb->setViewport({0, 0, float(size.width()), float(size.height())});
    const QRhiCommandBuffer::VertexInput input(vertexBuffer.get(), 0);
    cb->setVertexInput(0, 1, &input);
    cb->draw(int(vertices.size()));
    cb->endPass();
}

QImage normalizedReadback(const QRhiReadbackResult &result, bool yUp) {
    require(result.format == QRhiTexture::RGBA8, "Unexpected readback format");
    require(result.pixelSize.width() > 0 && result.pixelSize.height() > 0 &&
            result.data.size() == result.pixelSize.width() * result.pixelSize.height() * 4,
            "Incomplete readback");
    // This pipeline explicitly produces premultiplied UNORM RGBA. Verify that
    // contract before converting exactly once to straight-alpha PNG/readback.
    const auto *bytes = reinterpret_cast<const uchar *>(result.data.constData());
    for (qsizetype i = 0; i < result.data.size(); i += 4)
        require(bytes[i] <= bytes[i + 3] && bytes[i + 1] <= bytes[i + 3] && bytes[i + 2] <= bytes[i + 3],
                "Readback violates premultiplied alpha");
    QImage premultiplied(bytes, result.pixelSize.width(), result.pixelSize.height(),
                         result.pixelSize.width() * 4, QImage::Format_RGBA8888_Premultiplied);
    const QImage image = premultiplied.convertToFormat(QImage::Format_RGBA8888);
    QImage normalized = image.copy();
    if (yUp)
        for (int y = 0; y < image.height(); ++y)
            std::memcpy(normalized.scanLine(y), image.constScanLine(image.height() - 1 - y), image.width() * 4);
    return normalized;
}
