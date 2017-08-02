#include <gtest/gtest.h>

#include <dirent.h>
#ifndef WIN32
    #include <sys/types.h>
#endif

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <documentcanvas.h>
#include <painters/lccairopainter.tcc>
#include <file.h>
#include <drawables/gradientbackground.h>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options.hpp>
#include <fstream>

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
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = std::make_shared<lc::DocumentImpl>(_storageManager);
    auto _canvas = std::make_shared<LCViewer::DocumentCanvas>(_document);

    // Add backround
    auto _gradientBackground = std::make_shared<GradientBackground>(
            lc::Color(0x00, 0x00, 0x00),
            lc::Color(0x00, 0x00, 0x00)
    );
    _canvas->background().connect<GradientBackground, &GradientBackground::draw>(_gradientBackground.get());

    LcPainter* lcPainter = nullptr;
    _canvas->createPainterFunctor(
            [&](const unsigned int width, const unsigned int height) {
                if (lcPainter == nullptr) {
                    lcPainter = new LcCairoPainter<CairoPainter::backend::SVG>(imageWidth, imageHeight, nullptr);
                }

                return lcPainter;
            }
    );

    _canvas->deletePainterFunctor([&](LcPainter* painter) {
        if (painter != nullptr && lcPainter != nullptr) {
            delete painter;
            lcPainter = nullptr;
        }
    });

    _canvas->newDeviceSize(imageWidth, imageHeight);

    _canvas->render(
            [&](LcPainter&) {},
            [&](LcPainter&) {}
    );

    lc::File::open(_document, dxf, lc::File::LIBDXFRW);

    _canvas->setDisplayArea(lc::geo::Area(lc::geo::Coordinate(x, y), w, h));
    _canvas->render(
            [&](LcPainter&) {},
            [&](LcPainter&) {}
    );

    static_cast<LcCairoPainter<CairoPainter::backend::Image>*>(lcPainter)->writePNG(output);
}

bool checkRender(const std::string& image1, const std::string& image2, int tolerance) {
    GError* error1 = NULL;
    GError* error2 = NULL;

    auto pixbuf1 = gdk_pixbuf_new_from_file(image1.c_str(), &error1);
    auto pixbuf2 = gdk_pixbuf_new_from_file(image2.c_str(), &error2);

    if(error1) {
        std::cout << error1->message << std::endl;
        g_error_free(error1);
        return false;
    }
    if(error2) {
        std::cout << error2->message << std::endl;
        g_error_free(error2);
        return false;
    }

    if(gdk_pixbuf_get_width(pixbuf1) != gdk_pixbuf_get_width(pixbuf2) ||
       gdk_pixbuf_get_height(pixbuf1) != gdk_pixbuf_get_height(pixbuf2) ||
       gdk_pixbuf_get_n_channels(pixbuf1) != gdk_pixbuf_get_n_channels(pixbuf2)) {
        return false;
    }

    auto channelTolerance = 256 * (tolerance / 100);
    auto nbPixels = gdk_pixbuf_get_height(pixbuf1) * gdk_pixbuf_get_rowstride(pixbuf1);

    auto pixels1 = gdk_pixbuf_read_pixels(pixbuf1);
    auto pixels2 = gdk_pixbuf_read_pixels(pixbuf2);

    for(auto i = 0; i < nbPixels; i++) {
        auto pixel1 = pixels1[i];
        auto pixel2 = pixels2[i];
        if(pixel1 < pixel2 - channelTolerance || pixel1 > pixel2 + channelTolerance) {
            return false;
        }
    }

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
    int nbFiles = scandir("../unittest/rendering/res", &files, NULL, alphasort); //TODO: get correct folder
    if(nbFiles < 0) {
        perror("Error");
        FAIL() << "Cannot open rendering resources dir.";
    }

    unsigned int testNumber = 0;
    bool dxfFound = false;
    bool pngFound = false;
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
            pngFound = false;
            configFound = false;
        }

        if(strcmp(extension, "dxf") == 0) {
            dxfFound = true;
        }
        else if(strcmp(extension, "png") == 0) {
            pngFound = true;
        }
        else if(strcmp(extension, "cfg") == 0) {
            configFound = true;
        }

        if(dxfFound && pngFound && configFound) {
            auto base = std::string("../unittest/rendering/res/") + std::to_string(newNumber);
            auto expectedFile = base + ".png";
            auto dxfFile = base + ".dxf";
            auto resultFile = base + ".out";
            auto configFile = base + ".cfg";

            resetConfig(&imageW, &imageH, &x, &y, &w, &h, &tolerance);
            vm = po::variables_map();
            std::ifstream configStream(configFile.c_str());

            po::store(po::parse_config_file(configStream, desc), vm);
            po::notify(vm);

            render(dxfFile, resultFile, imageW, imageH, x, y, w, h);
            ASSERT_TRUE(checkRender(expectedFile, resultFile, tolerance)) << "Failed with " << expectedFile;

            dxfFound = false; //Prevent running the test more than once
            pngFound = false;
            configFound = false;
        }
    }
}