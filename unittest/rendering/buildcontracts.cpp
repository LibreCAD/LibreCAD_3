#if defined(_WIN32) && !defined(NOMINMAX)
#define NOMINMAX
#endif
#include "buildcontracts_config.h"
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#ifdef GetObject
#undef GetObject // Windows GDI macro must not rename RapidJSON's member API.
#endif
#else
#include <sys/utsname.h>
#endif

namespace {
namespace fs = boost::filesystem;
namespace json = rapidjson;
void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
fs::path path(const std::string& text) {
#ifdef _WIN32
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), int(text.size()), nullptr, 0);
    check(size > 0, "Invalid UTF-8 path");
    std::wstring wide(size, L'\0');
    check(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), int(text.size()), &wide[0], size) == size,
          "Cannot decode UTF-8 path");
    return fs::path(wide);
#else
    return fs::path(text);
#endif
}
fs::path runRoot() {
#ifdef _WIN32
    const auto size = GetEnvironmentVariableW(L"LC_BUILD_CONTRACT_RUN", nullptr, 0);
    check(size > 1 && size < 32768, "Missing build-contract run directory");
    std::wstring value(size, L'\0');
    const auto count = GetEnvironmentVariableW(L"LC_BUILD_CONTRACT_RUN", &value[0], size);
    check(count > 0 && count < size, "Invalid build-contract run directory");
    value.resize(count); return fs::path(value);
#else
    const auto* value = std::getenv("LC_BUILD_CONTRACT_RUN");
    check(value && *value, "Missing build-contract run directory"); return path(value);
#endif
}
std::string read(const fs::path& file, std::size_t limit = 4 * 1024 * 1024) {
    check(fs::is_regular_file(file) && !fs::is_symlink(file) && fs::file_size(file) <= limit,
          "Missing, linked or oversized build artifact");
    fs::ifstream input(file, std::ios::binary);
    check(bool(input), "Cannot read build artifact");
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
void unique(const json::Value& value, unsigned depth = 0) {
    check(depth < 32, "JSON nesting limit");
    if (value.IsObject()) {
        std::set<std::string> names;
        for (const auto& member : value.GetObject()) {
            check(names.insert(member.name.GetString()).second, "Duplicate JSON field"); unique(member.value, depth + 1);
        }
    } else if (value.IsArray()) for (const auto& item : value.GetArray()) unique(item, depth + 1);
}
json::Document parse(const std::string& text) {
    json::Document value; value.Parse<json::kParseValidateEncodingFlag>(text.data(), text.size());
    check(!value.HasParseError() && value.IsObject(), "Invalid build JSON"); unique(value); return value;
}
const json::Value& field(const json::Value& value, const char* key) {
    check(value.IsObject() && value.HasMember(key), "Missing build JSON field"); return value[key];
}
std::string text(const json::Value& value) { check(value.IsString(), "Expected JSON text"); return value.GetString(); }
int integer(const json::Value& value) { check(value.IsInt(), "Expected JSON integer"); return value.GetInt(); }
const json::Value& array(const json::Value& value) { check(value.IsArray(), "Expected JSON array"); return value; }
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return char(std::tolower(c)); }); return s;
}
std::string architecture() {
#if defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#else
    return "unknown";
#endif
}
std::string hostArchitecture() {
#ifdef _WIN32
    SYSTEM_INFO info; GetNativeSystemInfo(&info);
    return info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "x86_64" :
        info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64 ? "arm64" : "unknown";
#else
    struct utsname info; check(uname(&info) == 0, "Cannot query native architecture");
    const auto name = lower(info.machine); return name == "aarch64" ? "arm64" : name;
#endif
}
std::vector<fs::path> indices(const fs::path& build) {
    std::vector<fs::path> result;
    const auto reply = build / ".cmake/api/v1/reply";
    if (fs::exists(reply)) for (const auto& entry : fs::directory_iterator(reply)) {
        const auto name = entry.path().filename().string();
        if (name.compare(0, 6, "index-") == 0 && entry.path().extension() == ".json") result.push_back(entry.path());
    }
    return result;
}
json::Document reference(const fs::path& reply, const json::Value& ref) {
    const auto name = text(field(ref, "jsonFile"));
    check(fs::path(name).filename().string() == name, "Escaping File API reference");
    return parse(read(reply / path(name)));
}
struct Target { std::string type; bool graphics; };
std::map<std::string, Target> targets(const fs::path& build) {
    const auto files = indices(build); check(files.size() == 1, "Missing/ambiguous File API index");
    const auto reply = files[0].parent_path(); const auto index = parse(read(files[0]));
    auto model = reference(reply, field(field(field(index, "reply"), "client-renderer-build"), "codemodel-v2"));
    check(text(field(model, "kind")) == "codemodel" && integer(field(field(model, "version"), "major")) == 2,
          "Unexpected File API codemodel");
    check(fs::equivalent(path(text(field(field(model, "paths"), "source"))), path(build_contract::source)) &&
          fs::equivalent(path(text(field(field(model, "paths"), "build"))), build), "Foreign File API tree");
    const auto& configs = field(model, "configurations"); check(configs.IsArray(), "Missing File API configurations");
    const json::Value* config = nullptr;
    for (const auto& c : configs.GetArray()) if (text(field(c, "name")) == "Release") {
        check(!config, "Duplicate Release configuration"); config = &c;
    }
    check(config, "Missing Release configuration");
    std::map<std::string, Target> result;
    for (const auto& ref : array(field(*config, "targets")).GetArray()) {
        auto target = reference(reply, ref); const auto name = text(field(target, "name"));
        bool graphics = name == "lcviewer_opengl" || name == "lcrenderer_rhi" || name == "lcrenderertest" ||
            name == "librecad" || name == "lcui" || name == "luacmdinterface";
        if (target.HasMember("sources")) for (const auto& source : array(target["sources"]).GetArray()) {
            const auto p = lower(text(field(source, "path")));
            graphics |= p.find("painters/opengl/") != std::string::npos || p.find("lcrenderer/rhi/") != std::string::npos;
        }
        if (target.HasMember("link")) for (const auto& fragment : array(field(target["link"], "commandFragments")).GetArray()) {
            const auto p = lower(text(field(fragment, "fragment")));
            for (const auto* token : {"qt5", "qt6", "opengl", "glew", "glfw", "vulkan"}) graphics |= p.find(token) != std::string::npos;
        }
        check(name == text(field(ref, "name")) && result.emplace(name, Target{text(field(target, "type")), graphics}).second,
              "Duplicate/misbound File API target");
    }
    return result;
}
std::pair<int, int> discoveries(const fs::path& trace, bool jsonFormat) {
    const auto raw = read(trace, 64 * 1024 * 1024); std::istringstream stream(raw); std::string line;
    if (jsonFormat) {
        check(bool(std::getline(stream, line)), "Missing CMake trace header");
        const auto header = parse(line); check(integer(field(field(header, "version"), "major")) == 1, "Wrong CMake trace version");
    }
    const std::regex human(R"(^.+\([0-9]+\):[ \t]+([A-Za-z_][A-Za-z0-9_]*)\([ \t]*([A-Za-z0-9_]+).*$)");
    int qt = 0, legacy = 0, packages = 0;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        check(line.size() < 2 * 1024 * 1024, "Invalid trace line");
        std::string package;
        if (jsonFormat) {
            const auto command = parse(line);
            if (lower(text(field(command, "cmd"))) != "find_package") continue;
            const auto& args = array(field(command, "args")); check(!args.Empty(), "Invalid find_package trace");
            package = lower(text(args[0]));
        } else {
            std::smatch match;
            if (!std::regex_match(line, match, human) || lower(match[1].str()) != "find_package") continue;
            package = lower(match[2].str());
        }
        ++packages;
        qt += package.compare(0, 3, "qt5") == 0 || package.compare(0, 3, "qt6") == 0;
        legacy += package == "opengl" || package == "glew" || package == "glfw" || package == "glfw3" || package == "glm";
    }
    check(packages > 0, "CMake trace did not observe the project's required package discovery");
    return {qt, legacy};
}
class RenderBuildContractTest : public ::testing::Test {
protected:
    fs::path root;
    json::Document corpus;
    void SetUp() override {
        root = runRoot();
        const auto raw = read(path(build_contract::fixture));
        check(raw == read(path(build_contract::pinnedFixture)), "Fixture differs from the configured copy");
        corpus = parse(raw);
        check(integer(field(corpus, "schema_version")) == 1 &&
              text(field(corpus, "evidence_kind")) == "renderer-contract-inputs", "Wrong contract corpus");
        RecordProperty("build_fixture_sha256", build_contract::fixtureHash);
        RecordProperty("build_sources", build_contract::sources);
        RecordProperty("build_arch", architecture()); RecordProperty("host_arch", hostArchitecture());
    }
    void execute(const char* variant) {
        const auto directory = root / variant, build = directory / "build";
        const auto& body = field(field(field(corpus, "datasets"), "cpu-build-options"), variant);
        const auto& inputs = field(body, "inputs");
        const std::map<std::string, std::string> flags{{"rhi", "WITH_RHI_RENDERER"}, {"legacy", "WITH_LEGACY_OPENGL_RENDERER"},
            {"ui", "WITH_QT_UI"}, {"cli", "WITH_LUACMDINTERFACE"}, {"python", "WITH_PYTHONSCRIPT"}, {"tests-enabled", "WITH_UNITTESTS"}};
        const std::set<std::string> wanted = std::string(variant) == "ordinary" ? std::set<std::string>{"rhi","legacy","ui","cli","python"} :
            std::string(variant) == "empty" ? std::set<std::string>{"rhi","legacy","ui","cli","tests-enabled"} : std::set<std::string>{"rhi","legacy","ui"};
        check(inputs.IsObject() && inputs.MemberCount() == wanted.size(), "Unknown/missing CPU build inputs");
        const auto preflight = parse(read(build / "renderer-preflight.json"));
        check(integer(field(preflight, "schema_version")) == 1 && text(field(preflight, "evidence_kind")) == "renderer-option-preflight",
              "Invalid renderer option preflight");
        for (const auto& input : inputs.GetObject()) {
            const std::string key = input.name.GetString(); check(wanted.count(key), "Unconsumed build input");
            const auto& v = input.value;
            check(text(field(v, "type")) == "boolean" && text(field(v, "unit")) == "none" && field(v, "value").IsBool(), "Wrong build input type");
            const auto& actual = field(field(preflight, "options"), flags.at(key).c_str());
            ASSERT_TRUE(actual.IsBool()); EXPECT_EQ(field(v, "value").GetBool(), actual.GetBool()) << key;
        }
        const auto status = parse(read(directory / "result.json"));
        const auto& code = field(status, "exit_code"); check(code.IsInt(), "Missing CMake exit status");
        json::Document observed; observed.SetObject(); auto& a = observed.GetAllocator();
        if (std::string(variant) == "limit") {
            EXPECT_GT(code.GetInt(), 0);
            const auto log = read(directory / "stderr.log") + read(directory / "stdout.log");
            const auto marker = log.find("[no-renderer-for-host]");
            const auto diagnostic = marker == std::string::npos ? "" : log.substr(marker + 1, 20);
            observed.AddMember("diagnostic", json::Value(diagnostic.c_str(), a), a);
            observed.AddMember("accepted-output-count", int(indices(build).size()), a);
        } else {
            ASSERT_EQ(0, code.GetInt()); const auto graph = targets(build);
            if (std::string(variant) == "ordinary") {
                const auto format = text(field(status, "trace_format"));
                check(format == "json-v1" || format == "human", "Unknown CMake trace format");
                const auto counts = discoveries(directory / (format == "json-v1" ? "trace.jsonl" : "stderr.log"), format == "json-v1");
                observed.AddMember("qt-discovery-count", counts.first, a);
                observed.AddMember("legacy-discovery-count", counts.second, a);
                const auto cpu = graph.find("lcbuildcontracttest");
                observed.AddMember("cpu-contract-target-exists", cpu != graph.end() && cpu->second.type == "EXECUTABLE", a);
            } else {
                int count = 0;
                for (const auto& target : graph) count += target.second.graphics;
                observed.AddMember("gpu-target-count", count, a);
                EXPECT_EQ(0u, graph.count("lcbuildcontracttest")); EXPECT_EQ(0u, graph.count("lcunittest"));
            }
        }
        const auto& assertions = field(body, "assertions"); check(assertions.IsArray() && assertions.Size() == observed.MemberCount(), "Unconsumed assertions");
        for (const auto& assertion : assertions.GetArray()) {
            const auto key = text(field(assertion, "observed")); const auto& expected = field(field(assertion, "expected"), "value");
            EXPECT_EQ(expected, field(observed, key.c_str())) << key;
        }
        json::StringBuffer buffer; json::Writer<json::StringBuffer> writer(buffer); observed.Accept(writer);
        RecordProperty("build_case", std::string("cpu-build-options.") + variant);
        RecordProperty("build_observations", buffer.GetString());
    }
};
TEST_F(RenderBuildContractTest, CpuBuildOptionsOrdinary) { execute("ordinary"); }
TEST_F(RenderBuildContractTest, CpuBuildOptionsEmpty) { execute("empty"); }
TEST_F(RenderBuildContractTest, CpuBuildOptionsLimit) { execute("limit"); }
}
int main(int argc, char** argv) { ::testing::InitGoogleTest(&argc, argv); return RUN_ALL_TESTS(); }
