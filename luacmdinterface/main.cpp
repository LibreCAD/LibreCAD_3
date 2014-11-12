#include <cad/dochelpers/documentimpl.h>


#include <cad/primitive/circle.h>
#include <cad/primitive/coordinate.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/operations/builder.h>
#include <documentcanvas.h>
#include <lccairopainter.h>
#include <drawitems/gradientbackground.h>
#include <cad/dochelpers/undomanagerimpl.h>
#include <lcadluascript.h>
#include <string>
#include <curl/curl.h>
#include "boost/program_options.hpp"
namespace po = boost::program_options;



static char const *const DEFAULT_OUT_FILENAME = "out.png";
static const int DEFAULT_IMAGE_WIDTH = 400;
static const int DEFAULT_IMAGE_HEIGHT = 400;

static std::string readBuffer;
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    readBuffer.append((char *) contents, realsize);
    return realsize;
}

std::string loadFile(std::string url) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        /* example.com is redirected, so we tell libcurl to follow redirection */
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
        return readBuffer;
    } else {
        return "";
    }
}

int main(int argc, char **argv) {
    int width = DEFAULT_IMAGE_WIDTH;
    int height = DEFAULT_IMAGE_HEIGHT;
    std::string fIn = "";
    std::string fOut = DEFAULT_OUT_FILENAME;

    // Read CMD options
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("width,w", po::value<int>(&width), "(optional) Set output image width, example -w 350")
            ("height,h", po::value<int>(&height), "(optional) Set output image height, example -h 200")
            ("ifile,i", po::value<std::string>(&fIn), "(required) Set LUA input file name, example: -i file:myFile.lua")
            ("ofile,o", po::value<std::string>(&fOut), "(optional) Set output png filename, example -o out.png");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (width < 0) {
        std::cerr << "Width must be > 0" << std::endl;
    }
    if (height < 0) {
        std::cerr << "Height must be > 0" << std::endl;
    }
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }
    if (fIn.size()==0) {
        std::cerr << "Input filename cannot be empty" << std::endl;
        std::cout << desc << "\n";
        return 1;
    }

    // Create Librecad document
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto _canvas = new DocumentCanvas(_document);

    // Add backround
    _canvas->addBackgroundItem(std::shared_ptr<LCVDrawItem>(new GradientBackground(lc::Color(0x90, 0x90, 0x90), lc::Color(0x00, 0x00, 0x00))));

    // Seup a painter fo the document
    LcCairoPainter *lcPainter;
    _canvas->createPainterFunctor(
            [&](const unsigned int width, const unsigned int height) {
                if (lcPainter == NULL) {
                    lcPainter = LcCairoPainter::createPainter(width, height);
                    lcPainter->clear(1., 1., 1., 0.0);

                }
                return lcPainter;
            });

    _canvas->deletePainterFunctor([&]
            (LcPainter *painter) {
        if (lcPainter != NULL) {
            delete painter;
            lcPainter = NULL;
        }
    });

    // Set device width/height
    _canvas->newDeviceSize(width, height);


    // Render Lua Code
    LCadLuaScript luaScript(_document, _storageManager, true);
    std::string luaCode = loadFile(fIn);
    if (luaCode.size() != 0) {
        luaScript.run(luaCode);
    } else {
        std::cerr << "No lua code was loaded" << std::endl;
        return 1;
    }

    _canvas->autoScale();
    _canvas->render([&](
            LcPainter *lcPainter
    ) {
    }, [&](
            LcPainter *lcPainter
    ) {
    });


    lcPainter->writePNG(fOut);

    return 0;
}


