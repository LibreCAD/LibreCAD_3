#include "renderer.h"
#include "shader_p.h"
#include <QFile>
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QtGui/private/qshaderdescription_p.h>
#include <gtest/gtest.h>
#include <cmath>
#include <cstdlib>
#include <future>
#include <limits>
#include <algorithm>
#include <QFileInfo>
#include <QSet>
#include <QSysInfo>
#ifdef Q_OS_MACOS
#include <mach-o/dyld.h>
#elif defined(Q_OS_WIN)
#include <QtCore/qt_windows.h>
#endif

namespace {
using lc::rhi::Api;
using lc::viewer::render::Frame;
using lc::viewer::render::Snapshot;
std::unique_ptr<QGuiApplication> app;
int appArgc = 1;
char appName[] = "lcrenderertest";
char* appArgv[] = {appName, nullptr};
#ifdef Q_OS_MACOS
const auto nativeApi = Api::Metal;
const char* apiName = "metal";
#else
const auto nativeApi = Api::Vulkan;
const char* apiName = "vulkan";
#endif
Frame ordinary(int width = 65, int height = 49) {
    Frame f; f.width = width; f.height = height; f.clear = {{0, 0, 1, 1}};
    f.vertices = {{4, 4, 1, 0, 0, 1}, {float(width - 4), 4, 1, 0, 0, 1},
                   {4, float(height - 4), 1, 0, 0, 1}};
    return f;
}
void pixel(const Snapshot& s, int x, int y, std::array<int, 4> expected) {
    ASSERT_EQ(std::size_t(s.width) * s.height * 4, s.rgba.size());
    for (int c = 0; c < 4; ++c)
        EXPECT_NEAR(expected[c], s.rgba[(std::size_t(y) * s.width + x) * 4 + c], 2) << x << ',' << y << ',' << c;
}
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
#include "provenance.h"
QByteArray contractBytes(const QString& path) {
    QFile file(path);
    check(file.open(QIODevice::ReadOnly) && file.size() > 0 && file.size() <= 2 * 1024 * 1024,
          "Missing/oversized R1 contract fixture");
    return file.readAll();
}
QJsonObject checkedContracts(const QByteArray& bytes) {
    check(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex() == LC_RENDERER_CONTRACT_SHA256,
          "R1 contract fixture differs from the compiled input");
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(bytes, &error);
    check(error.error == QJsonParseError::NoError && document.isObject(), "Invalid R1 contract JSON");
    const auto root = document.object();
    check(root.value("schema_version").toInt() == 1 && root.value("evidence_kind") == "renderer-contract-inputs" &&
          root.value("datasets").isObject(), "Invalid R1 contract header");
    const auto dataset = root.value("datasets").toObject().value("rhi-build-options");
    check(dataset.isObject(), "Missing R1 RHI dataset");
    return dataset.toObject();
}
QJsonObject contracts() {
    const auto overridePath = qgetenv("LC_RENDERER_CONTRACT_PATH");
    return checkedContracts(contractBytes(overridePath.isEmpty() ? QString::fromUtf8(LC_RENDERER_CONTRACT_PATH)
                                                                 : QString::fromUtf8(overridePath)));
}
QJsonValue typed(const QJsonValue& value, const char* type, const char* unit) {
    check(value.isObject(), "Missing typed R1 value");
    const auto object = value.toObject();
    check(object.keys() == QStringList({"type", "unit", "value"}) && object.value("type") == type &&
          object.value("unit") == unit, "Wrong R1 value type/unit");
    return object.value("value");
}
int integer(const QJsonValue& value, int minimum = 0, int maximum = lc::viewer::render::MaxFrameDimension) {
    const double v = value.toDouble(std::numeric_limits<double>::quiet_NaN());
    check(value.isDouble() && std::isfinite(v) && std::floor(v) == v && v >= minimum && v <= maximum,
          "Invalid bounded R1 integer");
    return int(v);
}
class ContractCase {
public:
    ContractCase(const QJsonObject& dataset, const char* variant) : body(dataset.value(variant).toObject()), name(variant) {
        check(body.value("inputs").isObject() && body.value("assertions").isArray(), "Invalid R1 case");
    }
    void keys(QStringList expected) const {
        std::sort(expected.begin(), expected.end());
        check(body.value("inputs").toObject().keys() == expected, "Unconsumed/unknown R1 input");
    }
    QJsonValue input(const char* key, const char* type, const char* unit) const {
        return typed(body.value("inputs").toObject().value(key), type, unit);
    }
    QJsonValue expected(const char* key, const char* type, const char* unit) const {
        for (const auto& assertion : body.value("assertions").toArray()) {
            const auto object = assertion.toObject();
            if (object.value("observed") == key) return typed(object.value("expected"), type, unit);
        }
        throw std::runtime_error("Missing R1 assertion");
    }
    Frame frame() const {
        const auto size = input("size", "integers", "physical-pixel").toArray();
        check(size.size() == 2, "R1 target requires width/height");
        Frame f; f.width = integer(size[0]); f.height = integer(size[1]);
        return f;
    }
    Frame quad() const {
        keys({"size", "vertices", "indices", "rgba", "operations", "sample-count", "format",
              "background-rgba", "output-origin", "dpr"});
        check(input("operations", "strings", "none").toArray() == QJsonArray({"create-qualified-rhi",
              "clear-black", "upload-quad", "draw-indexed", "completed-readback"}), "Unsupported R1 operation sequence");
        check(integer(input("sample-count", "integer", "count")) == 1 &&
              input("format", "text", "none") == "RGBA8" &&
              input("output-origin", "text", "none") == "top-left" &&
              input("dpr", "number", "ratio").isDouble() &&
              input("dpr", "number", "ratio").toDouble() == 1, "Unsupported R1 target convention");
        auto f = frame();
        const auto rgba = input("rgba", "bytes", "8bit-channel").toArray();
        const auto background = input("background-rgba", "bytes", "8bit-channel").toArray();
        check(rgba.size() == 4 && background == QJsonArray({0, 0, 0, 255}), "Invalid R1 clear/color");
        std::array<float, 4> color;
        for (int c = 0; c < 4; ++c) {
            color[c] = integer(rgba[c], 0, 255) / 255.0f;
            f.clear[c] = integer(background[c], 0, 255) / 255.0f;
        }
        const auto points = input("vertices", "points2", "physical-pixel").toArray();
        check(points.size() == 4, "R1 quad requires four vertices");
        for (const auto& point : points) {
            const auto xy = point.toArray();
            check(xy.size() == 2 && xy[0].isDouble() && xy[1].isDouble(), "Invalid R1 vertex");
            const double x = xy[0].toDouble(), y = xy[1].toDouble();
            check(std::isfinite(x) && std::isfinite(y) && std::abs(x) <= 1e8 && std::abs(y) <= 1e8,
                  "Invalid R1 coordinate");
            f.vertices.push_back({float(x), float(y), color[0], color[1], color[2], color[3]});
        }
        const auto indices = input("indices", "integers", "count").toArray();
        check(indices.size() == 6, "R1 quad requires six indices");
        for (const auto& index : indices) f.indices.push_back(std::uint32_t(integer(index, 0, 3)));
        lc::viewer::render::validateFrame(f);
        return f;
    }
    int policyLimit() const {
        keys({"texture-limit", "requested-width", "limit-kind"});
        check(input("limit-kind", "text", "none") == "injected-renderer-policy", "Wrong R1 limit kind");
        return integer(input("texture-limit", "integer", "physical-pixel"), 1);
    }
    void record(QJsonObject observations) const {
        ::testing::Test::RecordProperty("rhi_case", "rhi-build-options." + name);
        ::testing::Test::RecordProperty("rhi_observations", QJsonDocument(observations).toJson(QJsonDocument::Compact).toStdString());
    }
private:
    QJsonObject body;
    std::string name;
};
QJsonArray bytes(const Snapshot& snapshot) {
    QJsonArray result;
    for (auto value : snapshot.rgba) result.append(int(value));
    return result;
}
QByteArray vertexBytes() {
    lc::rhi::detail::initializeResources();
    QFile file(":/librecad/rhi/triangles.vert.qsb");
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Missing test shader resource");
    return file.readAll();
}
class RenderBuildTest : public ::testing::Test {
protected:
    std::unique_ptr<lc::rhi::OffscreenRenderer> renderer;
    QJsonObject dataset;
    lc::rhi::DeviceInfo info;
    void SetUp() override {
        const char* expectedApi = std::getenv("LC_RENDERER_EXPECT_API");
        const char* expectedDevice = std::getenv("LC_RENDERER_EXPECT_DEVICE");
        ASSERT_NE(nullptr, expectedApi) << "Explicit qualification API is required";
        ASSERT_NE(nullptr, expectedDevice) << "Explicit software/hardware classification is required";
        ASSERT_STREQ(apiName, expectedApi);
        ASSERT_TRUE(std::string(expectedDevice) == "software" || std::string(expectedDevice) == "hardware");
        dataset = contracts(); // Reject missing/changed fixture bytes before native device creation.
        const std::string test = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        const int limit = test == "RhiLimit" || test == "TexturePolicyRecovery"
            ? ContractCase(dataset, "limit").policyLimit() : lc::viewer::render::MaxFrameDimension;
        if (!app) app.reset(new QGuiApplication(appArgc, appArgv));
        renderer = lc::rhi::createOffscreenRenderer(nativeApi, limit);
        info = renderer->deviceInfo();
        ASSERT_EQ(nativeApi, info.api);
        ASSERT_EQ(expectedDevice, info.deviceClass);
        ASSERT_FALSE(info.name.empty());
        ASSERT_EQ(64u, info.shaderHash.size());
        RecordProperty("rhi_api", apiName); RecordProperty("rhi_device", info.name);
        RecordProperty("rhi_device_class", info.deviceClass); RecordProperty("rhi_qt", info.qtVersion);
        RecordProperty("rhi_shader_sha256", info.shaderHash);
        RecordProperty("rhi_fixture_sha256", LC_RENDERER_CONTRACT_SHA256);
        RecordProperty("rhi_texture_limit", info.textureLimit);
        RecordProperty("rhi_texture_size_min", info.textureSizeMin);
        RecordProperty("rhi_texture_size_max", info.textureSizeMax);
        RecordProperty("rhi_qpa", QGuiApplication::platformName().toStdString());
        RecordProperty("rhi_arch", QSysInfo::currentCpuArchitecture().toStdString());
        RecordProperty("rhi_build_arch", QSysInfo::buildCpuArchitecture().toStdString());
        RecordProperty("rhi_qt_modules", QJsonDocument(rhiQtModules()).toJson(QJsonDocument::Compact).toStdString());
        QFile build(":/librecad/rhi/test-build.json");
        ASSERT_TRUE(build.open(QIODevice::ReadOnly));
        RecordProperty("rhi_build", build.readAll().toStdString());
    }
};
TEST_F(RenderBuildTest, RhiOrdinary) {
    const ContractCase c(dataset, "ordinary");
    const auto frame = c.quad();
    const auto result = renderer->render(frame);
    ASSERT_EQ(frame.width, result.width); ASSERT_EQ(frame.height, result.height);
    ASSERT_TRUE(result.targetReady);
    EXPECT_EQ(integer(c.expected("draw-count", "integer", "count")), int(result.drawCount));
    EXPECT_EQ(1u, result.indexedDrawCount);
    EXPECT_EQ(c.expected("normalized-rgba", "bytes", "8bit-channel"), bytes(result));
    EXPECT_EQ("backend", c.expected("actual-backend", "configuration", "none"));
    RecordProperty("rhi_indexed_draw_count", result.indexedDrawCount);
    c.record({{"normalized-rgba", bytes(result)}, {"draw-count", int(result.drawCount)},
              {"actual-backend", info.api == Api::Metal ? "metal" : "vulkan"}});
}
TEST_F(RenderBuildTest, RhiEmpty) {
    const ContractCase c(dataset, "empty");
    c.keys({"size"});
    const auto result = renderer->render(c.frame());
    EXPECT_EQ(integer(c.expected("draw-count", "integer", "count")), int(result.drawCount));
    const auto ready = c.expected("target-ready", "boolean", "none");
    ASSERT_TRUE(ready.isBool());
    EXPECT_EQ(ready.toBool(), result.targetReady);
    EXPECT_TRUE(result.rgba.empty());
    EXPECT_EQ(0u, result.indexedDrawCount);
    c.record({{"draw-count", int(result.drawCount)}, {"target-ready", result.targetReady}});
}
TEST_F(RenderBuildTest, RhiLimit) {
    const ContractCase c(dataset, "limit");
    const int limit = c.policyLimit();
    ASSERT_EQ(limit, info.textureLimit);
    ASSERT_GT(info.textureSizeMax, limit); // Exercise policy, not physical allocation exhaustion.
    Frame f; f.width = integer(c.input("requested-width", "integer", "physical-pixel"), 1); f.height = 1;
    int accepted = 0;
    std::string diagnostic;
    try {
        const auto result = renderer->render(f);
        if (result.targetReady && !result.rgba.empty()) ++accepted;
        ADD_FAILURE() << "Over-budget frame was not rejected";
    } catch (const lc::rhi::TextureSizeLimitError& error) {
        diagnostic = error.code();
        EXPECT_EQ(limit, error.maximum());
    }
    EXPECT_EQ(c.expected("diagnostic", "text", "none").toString().toStdString(), diagnostic);
    EXPECT_EQ(integer(c.expected("accepted-output-count", "integer", "count")), accepted);
    c.record({{"diagnostic", QString::fromStdString(diagnostic)}, {"accepted-output-count", accepted}});
}
// Preserve the nonuniform orientation check separately from the fixture's full quad.
TEST_F(RenderBuildTest, TriangleOrientation) {
    const auto result = renderer->render(ordinary());
    ASSERT_EQ(65, result.width); ASSERT_EQ(49, result.height);
    pixel(result, 10, 10, {{255, 0, 0, 255}});
    pixel(result, 1, 1, {{0, 0, 255, 255}});
    pixel(result, 60, 44, {{0, 0, 255, 255}});
}
TEST_F(RenderBuildTest, SuspendAndResume) {
    const auto old = renderer->render(ordinary());
    auto suspended = renderer->render(Frame{});
    EXPECT_FALSE(suspended.targetReady);
    EXPECT_TRUE(suspended.rgba.empty());
    EXPECT_EQ(0u, suspended.drawCount);
    EXPECT_EQ(0u, suspended.indexedDrawCount);
    pixel(old, 10, 10, {{255, 0, 0, 255}});
    pixel(renderer->render(ordinary(97, 73)), 10, 10, {{255, 0, 0, 255}});
}
TEST_F(RenderBuildTest, IndexedBufferReuse) {
    renderer->render(ordinary());
    auto quad = ContractCase(dataset, "ordinary").quad();
    auto result = renderer->render(quad);
    EXPECT_EQ(1u, result.indexedDrawCount);
    pixel(result, 1, 1, {{255, 0, 0, 255}});
    const auto vertices = quad.vertices;
    quad.vertices.insert(quad.vertices.end(), vertices.begin(), vertices.end());
    const auto indices = quad.indices;
    for (auto i : indices) quad.indices.push_back(i + 4);
    result = renderer->render(quad); // Grow vertex and index allocations without reordering.
    EXPECT_EQ(1u, result.drawCount); EXPECT_EQ(1u, result.indexedDrawCount);
    pixel(result, 1, 1, {{255, 0, 0, 255}});
    result = renderer->render(ordinary()); // Clear the old index binding.
    EXPECT_EQ(0u, result.indexedDrawCount);
    pixel(result, 1, 1, {{0, 0, 255, 255}});
    quad.vertices[0].r = 0; quad.vertices[0].g = 1;
    EXPECT_TRUE(renderer->render(quad).targetReady); // Reuse both larger buffers.
}
TEST_F(RenderBuildTest, InvalidIndexRecovery) {
    auto quad = ContractCase(dataset, "ordinary").quad();
    quad.indices[0] = std::numeric_limits<std::uint32_t>::max();
    EXPECT_THROW(renderer->render(quad), std::invalid_argument);
    quad.indices[0] = 0;
    pixel(renderer->render(quad), 1, 1, {{255, 0, 0, 255}});
}
TEST_F(RenderBuildTest, TexturePolicyRecovery) {
    Frame f; f.width = info.textureLimit + 1; f.height = 1;
    EXPECT_THROW(renderer->render(f), lc::rhi::TextureSizeLimitError);
    pixel(renderer->render(ContractCase(dataset, "ordinary").quad()), 1, 1, {{255, 0, 0, 255}});
}
TEST_F(RenderBuildTest, ClearOnly) {
    Frame f; f.width = 5; f.height = 3; f.clear = {{0, 0, 1, 1}};
    const auto result = renderer->render(f);
    EXPECT_TRUE(result.targetReady);
    EXPECT_EQ(0u, result.drawCount); EXPECT_EQ(0u, result.indexedDrawCount);
    pixel(result, 2, 1, {{0, 0, 255, 255}});
}
TEST_F(RenderBuildTest, ResizeAndSnapshotLifetime) {
    const auto old = renderer->render(ordinary());
    auto resized = renderer->render(ordinary(97, 73));
    renderer.reset();
    pixel(old, 10, 10, {{255, 0, 0, 255}});
    pixel(resized, 10, 10, {{255, 0, 0, 255}});
    pixel(resized, 92, 68, {{0, 0, 255, 255}});
}
TEST_F(RenderBuildTest, PremultipliedAlpha) {
    auto f = ordinary(); f.clear = {{0, 0, 0.5f, 0.5f}};
    for (auto& v : f.vertices) { v.r = 0.5f; v.a = 0.5f; }
    const auto result = renderer->render(f);
    pixel(result, 10, 10, {{128, 0, 64, 191}});
    pixel(result, 1, 1, {{0, 0, 128, 128}});
}
TEST_F(RenderBuildTest, RejectsInvalidThenRecovers) {
    auto f = ordinary(); f.vertices[0].x = std::numeric_limits<float>::infinity();
    EXPECT_THROW(renderer->render(f), std::invalid_argument);
    pixel(renderer->render(ordinary()), 10, 10, {{255, 0, 0, 255}});
}
TEST_F(RenderBuildTest, OwnerThread) {
    auto other = std::async(std::launch::async, [&] {
        EXPECT_THROW(renderer->render(ordinary()), std::logic_error);
    });
    other.get();
    pixel(renderer->render(ordinary()), 10, 10, {{255, 0, 0, 255}});
}
TEST_F(RenderBuildTest, WrongBackendDoesNotFallback) {
    const auto wrong = nativeApi == Api::Metal ? Api::Vulkan : Api::Metal;
    EXPECT_THROW(lc::rhi::createOffscreenRenderer(wrong), std::invalid_argument);
    pixel(renderer->render(ordinary()), 10, 10, {{255, 0, 0, 255}});
}
TEST(RenderFactoryTest, InvalidPolicyBeforeDeviceCreation) {
    for (int limit : {-1, 0, lc::viewer::render::MaxFrameDimension + 1})
        EXPECT_THROW(lc::rhi::createOffscreenRenderer(nativeApi, limit), std::invalid_argument);
}
TEST(RenderContractTest, MissingOrChangedInput) {
    EXPECT_THROW(contractBytes(QStringLiteral("/missing-librecad-contract.json")), std::runtime_error);
    const auto bytes = contractBytes(QString::fromUtf8(LC_RENDERER_CONTRACT_PATH));
    EXPECT_NO_THROW(checkedContracts(bytes));
    EXPECT_THROW(checkedContracts(bytes + QByteArray(" ")), std::runtime_error);
}
TEST(RenderShaderTest, EmbeddedVariantsAndRegistration) {
    EXPECT_NO_THROW(lc::rhi::detail::checkedShader(vertexBytes(), QShader::VertexStage));
    EXPECT_NO_THROW(lc::rhi::detail::resourceShader(":/librecad/rhi/triangles.frag.qsb", QShader::FragmentStage));
}
TEST(RenderShaderTest, MissingAndCorrupt) {
    EXPECT_THROW(lc::rhi::detail::resourceShader(":/librecad/rhi/missing.qsb", QShader::VertexStage), std::runtime_error);
    EXPECT_THROW(lc::rhi::detail::checkedShader(QByteArray(), QShader::VertexStage), std::runtime_error);
    EXPECT_THROW(lc::rhi::detail::checkedShader(vertexBytes().left(8), QShader::VertexStage), std::runtime_error);
}
TEST(RenderShaderTest, WrongStage) {
    EXPECT_THROW(lc::rhi::detail::checkedShader(vertexBytes(), QShader::FragmentStage), std::runtime_error);
}
TEST(RenderShaderTest, MissingBackendVariant) {
    auto shader = QShader::fromSerialized(vertexBytes());
    shader.removeShader(QShaderKey(QShader::MslShader, QShaderVersion(12)));
    EXPECT_THROW(lc::rhi::detail::checkedShader(shader.serialized(), QShader::VertexStage), std::runtime_error);
}
TEST(RenderShaderTest, UnexpectedVariant) {
    auto shader = QShader::fromSerialized(vertexBytes());
    shader.setShader(QShaderKey(QShader::GlslShader, QShaderVersion(440)),
                     QShaderCode(QByteArray("not a permitted backend"), QByteArray("main")));
    EXPECT_THROW(lc::rhi::detail::checkedShader(shader.serialized(), QShader::VertexStage), std::runtime_error);
}
TEST(RenderShaderTest, ReflectionMismatch) {
    auto shader = QShader::fromSerialized(vertexBytes());
    auto description = shader.description();
    QShaderDescriptionPrivate::get(&description)->uniformBlocks[0].size = 60;
    shader.setDescription(description);
    EXPECT_THROW(lc::rhi::detail::checkedShader(shader.serialized(), QShader::VertexStage), std::runtime_error);
}
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    const int status = RUN_ALL_TESTS();
    app.reset(); // All owned renderers/test fixtures are already destroyed.
    return status;
}
