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

void render(const std::string& dxf, const std::string& output, unsigned int width, unsigned int height) {
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
                    lcPainter = new LcCairoPainter<CairoPainter::backend::SVG>(width, height, nullptr);
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

    _canvas->newDeviceSize(width, height);

    _canvas->render(
            [&](LcPainter&) {},
            [&](LcPainter&) {}
    );

    lc::File::open(_document, dxf, lc::File::LIBDXFRW);

    _canvas->autoScale();
    _canvas->render(
            [&](LcPainter&) {},
            [&](LcPainter&) {}
    );

    static_cast<LcCairoPainter<CairoPainter::backend::Image>*>(lcPainter)->writePNG(output);
}

bool checkRender(const std::string& image1, const std::string& image2) {
    GError* error1 = NULL;
    GError* error2 = NULL;

    auto pixbuf1 = gdk_pixbuf_new_from_file(image1.c_str(), &error1);
    auto pixbuf2 = gdk_pixbuf_new_from_file(image2.c_str(), &error2);

    if(error1) {
        g_error_free(error1);
        return false;
    }
    if(error2) {
        g_error_free(error2);
        return false;
    }

    if(gdk_pixbuf_get_width(pixbuf1) != gdk_pixbuf_get_width(pixbuf2) ||
       gdk_pixbuf_get_height(pixbuf1) != gdk_pixbuf_get_height(pixbuf2) ||
       gdk_pixbuf_get_n_channels(pixbuf1) != gdk_pixbuf_get_n_channels(pixbuf2)) {
        return false;
    }

    return (bool) g_bytes_equal(gdk_pixbuf_read_pixel_bytes(pixbuf1), gdk_pixbuf_read_pixel_bytes(pixbuf2));
}

TEST(RenderingTest, Test) {
    DIR* dir = opendir("../unittest/rendering/res"); //TODO: get correct folder
    if(!dir) {
        perror("Error");
        FAIL() << "Cannot open rendering resources dir.";
    }

    dirent* file = NULL;
    unsigned int testNumber = 0;
    bool dxfFound = false;
    bool pngFound = false;

    while ((file = readdir(dir)) != NULL) {
        unsigned int newNumber;
        char extension[256];

        if(sscanf(file->d_name, "%u.%s", &newNumber, extension) != 2) {
            continue;
        }

        if(newNumber != testNumber) {
            testNumber = newNumber;
            dxfFound = false;
            pngFound = false;
        }

        if(strcmp(extension, "dxf") == 0) {
            dxfFound = true;
        }
        else if(strcmp(extension, "png") == 0) {
            pngFound = true;
        }

        if(dxfFound && pngFound) {
            auto base = std::string("../unittest/rendering/res/") + std::to_string(newNumber);
            auto expectedFile = base + ".png";
            auto dxfFile = base + ".dxf";
            auto resultFile = base + ".out";

            render(dxfFile, resultFile, 100, 100); //TODO: get real size
            ASSERT_TRUE(checkRender(expectedFile, resultFile)) << "Failed with " << expectedFile;

            dxfFound = false; //Prevent running the test more than once
            pngFound = false;
        }
    }

    closedir(dir);
}