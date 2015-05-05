#include <cad/dochelpers/documentimpl.h>
#include <fstream>
#include <cstdio>

#include <cad/functions/intersect.h>

/*
#include <cad/geometry/geovector.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/line.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/primitive/spline.h>
#include <cad/primitive/point.h>
 */

#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/operations/builder.h>
#include <documentcanvas.h>
#include <lccairopainter.tcc>
#include <drawables/gradientbackground.h>
#include <cad/dochelpers/undomanagerimpl.h>
#include <lcadluascript.h>
#include <string>
#include <curl/curl.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
namespace po = boost::program_options;


static char const* const DEFAULT_OUT_FILENAME = "out.png";
static const int DEFAULT_IMAGE_WIDTH = 400;
static const int DEFAULT_IMAGE_HEIGHT = 400;

static std::string readBuffer;
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    readBuffer.append((char*) contents, realsize);
    return realsize;
}



std::string loadFile(std::string url) {
    CURL* curl;
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

std::ofstream ofile;
cairo_status_t write_func (void * closure, const unsigned char *data, uint length) {
    
    if (ofile.is_open())
       ofile.write((const char *)data, length);
    
    return CAIRO_STATUS_SUCCESS;
}

int main(int argc, char** argv) {
    int width = DEFAULT_IMAGE_WIDTH;
    int height = DEFAULT_IMAGE_HEIGHT;
    std::string fIn = "";
    std::string fOut = DEFAULT_OUT_FILENAME;
    std::string fType;

    // Read CMD options
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("width,w", po::value<int>(&width), "(optional) Set output image width, example -w 350")
    ("height,h", po::value<int>(&height), "(optional) Set output image height, example -h 200")
    ("ifile,i", po::value<std::string>(&fIn), "(required) Set LUA input file name, example: -i file:myFile.lua")
    ("ofile,o", po::value<std::string>(&fOut), "(optional) Set output filename, example -o out.png")
    ("otype,t", po::value<std::string>(&fType), "(optional) output file type, example -t svg");

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

    if (fIn.size() == 0) {
        std::cerr << "Input filename cannot be empty" << std::endl;
        std::cout << desc << "\n";
        return 1;
    }

    // Create Librecad document
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = std::make_shared<lc::DocumentImpl>(_storageManager);
    auto _canvas = std::make_shared<DocumentCanvas>(_document);

    // Add backround
    auto _gradientBackground = std::make_shared<GradientBackground>(lc::Color(0x90, 0x90, 0x90), lc::Color(0x00, 0x00, 0x00));
    _canvas->background().connect<GradientBackground, &GradientBackground::draw>(_gradientBackground.get());

    /* try to guess from file extension the output type */
    if (fType.empty()) {
        fType = boost::filesystem::extension(fOut);
        fType = fType.substr(fType.find_first_of(".")+1);
    }
        
    std::transform(fType.begin(), fType.end(), fType.begin(), ::tolower);
    LcPainter * lcPainter = nullptr;    
    ofile.open(fOut);
    
    using namespace CairoPainter;
    if (fType == "pdf")
        lcPainter = new LcCairoPainter<backend::PDF>(width, height, &write_func);
    else if (fType == "svg")
        lcPainter = new LcCairoPainter<backend::SVG>(width, height, &write_func);
    // cairo can print any surface to PNG
    else
        lcPainter = new LcCairoPainter<backend::SVG>(width, height, nullptr);
        

    _canvas->createPainterFunctor(
        [&](const unsigned int width, const unsigned int height) {
                lcPainter->clear(1., 1., 1., 0.0);
            return lcPainter;
    });

    _canvas->deletePainterFunctor([&]
    (LcPainter * painter) {
        if (lcPainter != nullptr) {
            delete painter;
            lcPainter = nullptr;
        }
    });

    // Set device width/height
    _canvas->newDeviceSize(width, height);


    // Render Lua Code
    LCadLuaScript luaScript(_document, false);
    std::string luaCode = loadFile(fIn);


    if (luaCode.size() != 0) {
        std::string out = luaScript.run(luaCode);

        if (out.size() > 0) {
            std::cerr << out << std::endl;
            return 2;
        }
    } else {
        std::cerr << "No lua code was loaded" << std::endl;
        return 1;
    }

    _canvas->autoScale();
    _canvas->render([&](LcPainter & lcPainter) {},
                    [&](LcPainter & lcPainter) {});


    if (fType == "png" || (fType != "pdf" && fType != "svg"))
        static_cast<LcCairoPainter<CairoPainter::backend::Image> *>(lcPainter)->writePNG(fOut);
    
    ofile.close();
    

    return 0;
}


