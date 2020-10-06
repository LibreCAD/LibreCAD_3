#include <GL/glew.h>

#include <gtest/gtest.h>
#include <dirent.h>

#ifndef WIN32
#include <sys/types.h>
#endif

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <documentcanvas.h>
#include <file.h>
#include <drawables/gradientbackground.h>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <GLFW/glfw3.h>
#include <painters/createpainter.h>

#define DEFAULT_IMAGE_WIDTH 100
#define DEFAULT_IMAGE_HEIGHT 100
#define DEFAULT_X 0
#define DEFAULT_Y 0
#define DEFAULT_W 100
#define DEFAULT_H 100
#define DEFAULT_TOLERANCE 0

namespace po = boost::program_options;

void resetConfig(int* imageWidth, int* imageHeight, int* x, int* y, int* w, int* h, int* tolerance) {
    *imageWidth = DEFAULT_IMAGE_WIDTH;
    *imageHeight = DEFAULT_IMAGE_HEIGHT;
    *x = DEFAULT_X;
    *y = DEFAULT_Y;
    *w = DEFAULT_W;
    *h = DEFAULT_H;
    *tolerance = DEFAULT_TOLERANCE;
}

void render(const std::string& dxf, const std::string& output, unsigned int imageWidth, unsigned int imageHeight,
            int x, int y, int w, int h) {
    auto _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto _document = std::make_shared<lc::storage::DocumentImpl>(_storageManager);
    auto _canvas = std::make_shared<lc::viewer::DocumentCanvas>(_document, [](double* x, double* y) {}); //MODEL viewer

    // Add background
    auto _gradientBackground = std::make_shared<lc::viewer::drawable::GradientBackground>(
                                   lc::Color(0x00, 0x00, 0x00),
                                   lc::Color(0x00, 0x00, 0x00)
                               );
    _canvas->background().connect<lc::viewer::drawable::GradientBackground, &lc::viewer::drawable::GradientBackground::draw>(_gradientBackground.get());

    lc::viewer::LcPainter* lcPainter;
    if(!glfwInit()) {
        LOG_ERROR << "Failed to initialize GLFW";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window;
    window = glfwCreateWindow(imageWidth, imageHeight, "LibreCAD", nullptr, nullptr);
    if(window == nullptr) {
        glfwTerminate();
        FAIL() << "Failed opening GLFW: " << std::endl;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK) {
        FAIL() << "GLEW Error: " << glewGetErrorString(err) << std::endl;
    }

    LOG_INFO << (char*) glGetString(GL_VERSION) << std::endl;

    lcPainter = lc::viewer::createOpenGLPainter(nullptr, imageWidth, imageHeight);

    lcPainter->create_resources();
    _canvas->setPainter(lcPainter);

    // Set device width/height
    _canvas->newDeviceSize(imageWidth, imageHeight);
    lcPainter->new_device_size(imageWidth, imageHeight);


    lc::persistence::File::open(_document, dxf, lc::persistence::File::LIBDXFRW);

    _canvas->setDisplayArea(*lcPainter, lc::geo::Area(lc::geo::Coordinate(x, y), w, h));
    _canvas->render(*lcPainter, lc::viewer::VIEWER_BACKGROUND);
    _canvas->render(*lcPainter, lc::viewer::VIEWER_DOCUMENT);
    _canvas->render(*lcPainter, lc::viewer::VIEWER_FOREGROUND);

    glfwSwapBuffers(window);

    FILE* out = fopen(output.c_str(), "wb");
    char* pixel_data = new char[3*imageWidth*imageHeight];
    short TGAhead[] = { 0, 2, 0, 0, 0, 0, static_cast<short>(imageWidth), static_cast<short>(imageHeight), 24 };

    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, imageWidth, imageHeight, GL_BGR, GL_UNSIGNED_BYTE, pixel_data);

    fwrite(&TGAhead,sizeof(TGAhead),1,out);
    fwrite(pixel_data, 3*imageWidth*imageHeight, 1, out);
    fclose(out);

    delete[] pixel_data;

    glfwDestroyWindow(window);
    delete lcPainter;
}

bool checkRender(const std::string& image1, const std::string& image2, float tolerance) {
    std::fstream f1;
    f1.open(image1, std::ios::in);
    if(f1.fail()) {
        std::cerr << "File " << image1 << " can't be opened" << std::endl;
        return false;
    }

    std::fstream f2;
    f2.open(image2, std::ios::in);
    if(f2.fail()) {
        std::cerr << "File " << image2 << " can't be opened" << std::endl;
        return false;
    }

    auto channelTolerance = 256.0 * (tolerance / 100.0);
    char c1 = '0';
    char c2 = '0';
    while((c1 != EOF) && (c2 != EOF)){
        c1 = f1.get();
        c2 = f2.get();

        if(c1 < c2 - channelTolerance || c1 > c2 + channelTolerance){
            return false;
        }

        if((c1 == EOF) ^ (c2 == EOF)) {
            return false;
        }
    }
    f1.close();
    f2.close();

    return true;
}

TEST(RenderingTest, Test) {
    int imageW;
    int imageH;
    int x;
    int y;
    int w;
    int h;
    int tolerance;

    resetConfig(&imageW, &imageH, &x, &y, &w, &h, &tolerance);

    po::variables_map vm;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("imageW", po::value<int>(&imageW), "Image width (in pixels)")
    ("imageH", po::value<int>(&imageH), "Image height (in pixels)")
    ("x", po::value<int>(&x), "Canvas base")
    ("y", po::value<int>(&y), "Canvas base")
    ("w", po::value<int>(&w), "Canvas width")
    ("h", po::value<int>(&h), "Canvas height")
    ("tolerance", po::value<int>(&tolerance), "Tolerance (between 0 and 100)");

    dirent** files = NULL;

    const char* resDir = SOURCE_DIR "/rendering/res/";
    std::cout << "Opening resources from " << resDir << std::endl;
    int nbFiles = scandir(resDir, &files, NULL, alphasort);
    if(nbFiles < 0) {
        perror("Error");
        FAIL() << "Cannot open rendering resources dir.";
    }

    unsigned int testNumber = 0;
    bool dxfFound = false;
    bool tgaFound = false;
    bool configFound = false;

    for(auto i = 0; i < nbFiles; i++) {
        unsigned int newNumber;
        char extension[256];

        if(sscanf(files[i]->d_name, "%u.%s", &newNumber, extension) != 2) {
            continue;
        }

        if(newNumber != testNumber) {
            testNumber = newNumber;
            dxfFound = false;
            tgaFound = false;
            configFound = false;
        }

        if(strcmp(extension, "dxf") == 0) {
            dxfFound = true;
        }
        else if(strcmp(extension, "tga") == 0) {
            tgaFound = true;
        }
        else if(strcmp(extension, "cfg") == 0) {
            configFound = true;
        }

        if(dxfFound && tgaFound && configFound) {
            auto base = std::string(resDir) + std::to_string(newNumber);
            auto expectedFile = base + ".tga";
            auto dxfFile = base + ".dxf";
            auto resultFile = base + ".out";
            auto configFile = base + ".cfg";

            resetConfig(&imageW, &imageH, &x, &y, &w, &h, &tolerance);
            vm = po::variables_map();
            std::ifstream configStream(configFile.c_str());

            po::store(po::parse_config_file(configStream, desc), vm);
            po::notify(vm);

            std::cout << "Running rendering test " << newNumber << std::endl;
            std::cout << "Size " << imageW << "*" << imageH << std::endl;
            std::cout << "Box " << x << ";" << y << " - " << w << "*" << h << std::endl;
            std::cout << "Tolerance " << tolerance << std::endl;

            render(dxfFile, resultFile, imageW, imageH, x, y, w, h);
            ASSERT_TRUE(checkRender(expectedFile, resultFile, tolerance)) << "Failed with " << expectedFile;

            dxfFound = false; //Prevent running the test more than once
            tgaFound = false;
            configFound = false;
        }
    }
}
