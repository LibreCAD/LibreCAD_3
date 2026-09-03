#include <lclua.h>

#include <cad/storage/documentimpl.h>
#include <fstream>

#include <cad/storage/storagemanagerimpl.h>
#include <documentcanvas.h>
#include <drawables/gradientbackground.h>
#include <curl/curl.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <managers/pluginmanager.h>
#include <managers/luacustomentitymanager.h>
#include <painters/createpainter.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "dispatch.h"

#ifdef LC_WITH_PYTHONSCRIPT
#include <lcpython.h>
#endif


namespace po = boost::program_options;

using LcPainter = lc::viewer::LcPainter;

static char const* const DEFAULT_OUT_FILENAME = "out.png";
static const int DEFAULT_IMAGE_WIDTH = 400;
static const int DEFAULT_IMAGE_HEIGHT = 400;

static std::string* readBuffer;
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void*) {
    size_t realsize = size * nmemb;
    readBuffer->append((char*) contents, realsize);
    return realsize;
}

std::string loadFile(const std::string& url) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl != nullptr) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return "";
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
        return *readBuffer;
    } else {
        return "";
    }
}

static FILE* openFileDialog(bool isOpening, const char* description, const char* mode) {
    std::string path;

    if(isOpening) {
        std::cout << "Enter path to open " << description << ": ";
    }
    else {
        std::cout << "Enter path to save " << description << ": ";
    }

    std::cin >> path;

    return fopen(path.c_str(), mode);
}

int main(int argc, char** argv) {
    int width = DEFAULT_IMAGE_WIDTH;
    int height = DEFAULT_IMAGE_HEIGHT;
    std::string fIn;
    std::string fOut = DEFAULT_OUT_FILENAME;
    std::string fType;
    readBuffer = new std::string;

    // Read CMD options
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("width,w", po::value<int>(&width), "(optional) Set output image width, example -w 350")
    ("height,h", po::value<int>(&height), "(optional) Set output image height, example -h 200")
    ("ifile,i", po::value<std::string>(&fIn), "(required) Set LUA input file name, example: -i file:myFile.lua")
    ("ofile,o", po::value<std::string>(&fOut), "(optional) Set output filename, example -o out.png")
    ("otype,t", po::value<std::string>(&fType), "(optional) output file type, example -t tga");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (width < 0) {
        std::cerr << "Width must be > 0" << std::endl;
    }

    if (height < 0) {
        std::cerr << "Height must be > 0" << std::endl;
    }

    if (vm.count("help") != 0u) {
        std::cout << desc << "\n";
        return 1;
    }

    if (fIn.empty()) {
        std::cerr << "Input filename cannot be empty" << std::endl;
        std::cout << desc << "\n";
        return 1;
    }

    // Create Librecad document
    auto _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto _document = std::make_shared<lc::storage::DocumentImpl>(_storageManager);
    auto _canvas = std::make_shared<lc::viewer::DocumentCanvas>(_document);

    // Add background
    auto _gradientBackground = std::make_shared<lc::viewer::drawable::GradientBackground>(lc::Color(0x90, 0x90, 0x90),
                               lc::Color(0x00, 0x00, 0x00));
    _canvas->background().connect<lc::viewer::drawable::GradientBackground, &lc::viewer::drawable::GradientBackground::draw>(_gradientBackground.get());

    /* try to guess from file extension the output type */
    if (fType.empty()) {
        fType = fOut.extension().string();
        if (!fType.empty() && fType[0] == '.')
            fType = fType.substr(1);
    }

    std::transform(fType.begin(), fType.end(), fType.begin(), ::tolower);

    LcPainter* lcPainter;
    if(!glfwInit()) {
        LOG_ERROR << "Failed to initialize GLFW";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window;
    window = glfwCreateWindow(width, height, "LibreCAD", nullptr, nullptr);
    if(window == nullptr) {
        const char* description;
        glfwGetError(&description);
        LOG_ERROR << "Failed opening GLFW: " << description << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK) {
        LOG_ERROR << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        exit(1);
    }
    if (!GLEW_VERSION_2_1) {
        LOG_ERROR << "OpenGL version 2.1 is not available" << std::endl;
        exit(1);
    }

    LOG_INFO << (char*) glGetString(GL_VERSION) << std::endl;

    lcPainter = lc::viewer::createOpenGLPainter(nullptr, width, height);

    lcPainter->create_resources();
    _canvas->setPainter(lcPainter);

    // Set device width/height
    _canvas->newDeviceSize(width, height);
    lcPainter->new_device_size(width, height);

    // ---- Runtime dispatch by URL path suffix (phase 2 slice 2.1) -----------
    // .lua → existing LCLua path (verbatim; Lua behavior is a hard plan
    //        constraint — preserve exactly).
    // .py  → new LCPython path, gated on LC_WITH_PYTHONSCRIPT so
    //        WITH_PYTHONSCRIPT=OFF builds still work.
    // Query/fragment are stripped before matching (dispatch.h) so
    // `file:test.py?flavor=old` picks Python rather than Unknown.
    const auto runtime = lc::cli::chooseScriptRuntime(fIn);
    if (runtime == lc::cli::ScriptRuntime::Unknown) {
        std::cerr << "Unrecognized script suffix (expected .lua or .py) in: "
                  << fIn << std::endl;
        return 1;
    }

    std::string code = loadFile(fIn);
    if (code.empty()) {
        std::cerr << "No script code was loaded" << std::endl;
        return 1;
    }

    if (runtime == lc::cli::ScriptRuntime::Lua) {
        // Verbatim from the pre-slice-2.1 path — Lua behavior unchanged.
        kaguya::State luaState;

        lc::lua::PluginManager pluginManager(luaState.state(), "cli");
        pluginManager.loadPlugins();

        auto lcLua = lc::lua::LCLua(luaState.state());
        lcLua.setF_openFileDialog(&openFileDialog);
        lcLua.addLuaLibs();
        lcLua.importLCKernel();
        lcLua.setDocument(_document);

        std::string out = lcLua.runString(code.c_str());
        if (!out.empty()) {
            std::cerr << out << std::endl;
            return 2;
        }
    }
#ifdef LC_WITH_PYTHONSCRIPT
    else if (runtime == lc::cli::ScriptRuntime::Python) {
        // Parity with the Lua path: PythonInit + fresh namespace +
        // setDocument. `document` is the ONLY name injected, matching
        // LCLua::setDocument's single-binding contract (plan sub-plan 2.2).
        // No plugins / no path.py in phase 2 — those land in phase 5.
        lc::python::LCPython lcpy;
        auto ns = lcpy.makeNamespace();
        lcpy.setDocument(ns, _document);

        const std::string out = lcpy.runString(code.c_str(), ns);
        if (!out.empty()) {
            // Same exit code contract as the Lua path (:199-203): script
            // error → stderr + exit 2.
            std::cerr << out << std::endl;
            return 2;
        }
    }
#else
    else if (runtime == lc::cli::ScriptRuntime::Python) {
        std::cerr << "Python scripting was not compiled in (rebuild with "
                     "WITH_PYTHONSCRIPT=ON): " << fIn << std::endl;
        return 1;
    }
#endif

    _canvas->autoScale(*lcPainter);
    lcPainter->clear(0,0,0);

    _canvas->render(*lcPainter, lc::viewer::VIEWER_BACKGROUND);
    _canvas->render(*lcPainter, lc::viewer::VIEWER_DOCUMENT);
    _canvas->render(*lcPainter, lc::viewer::VIEWER_FOREGROUND);

    glfwSwapBuffers(window);

    FILE* out = fopen(fOut.c_str(), "wb");
    char* pixel_data = new char[3*width*height];
    short TGAhead[] = { 0, 2, 0, 0, 0, 0, static_cast<short>(width), static_cast<short>(height), 24 };

    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixel_data);

    fwrite(&TGAhead,sizeof(TGAhead),1,out);
    fwrite(pixel_data, 3*width*height, 1, out);
    fclose(out);

    delete[] pixel_data;

    // Phase 6 PR-6.1 — was `LuaCustomEntityManager::getInstance().removePlugins()`.
    // In luacmdinterface this call happened at process exit (single-shot
    // CLI tool), so the behavior wasn't harmful the way the ~LuaInterface
    // call in lcUI was.  Kept the removal for consistency: the manager's
    // own static destructor clears its map at process exit, so this call
    // is redundant here and misleading (implies per-window cleanup which
    // was the source of the multi-window bug).
    glfwDestroyWindow(window);

    delete lcPainter;
    delete readBuffer;
    return 0;
}