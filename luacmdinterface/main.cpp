#include <cad/dochelpers/documentimpl.h>


#include <cad/primitive/circle.h>
#include <cad/primitive/coordinate.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/operations/builder.h>
#include <documentcanvas.h>
#include <lccairopainter.h>
#include <drawitems/gradientbackground.h>
#include <cad/dochelpers/undomanagerimpl.h>


int main(int argc, char** argv) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);
    auto _canvas = new DocumentCanvas(_document);

    _canvas->addBackgroundItem(std::shared_ptr<LCVDrawItem>(new GradientBackground(lc::Color(0x06, 0x35, 0x06), lc::Color(0x17, 0x15, 0x11))));

    LcCairoPainter* lcPainter;
    _canvas->createPainterFunctor(
    [&](const unsigned int width, const unsigned int height) {
        if (lcPainter == NULL) {
            lcPainter = LcCairoPainter::createPainter(width, height);
            lcPainter->clear(1., 1., 1., 0.0);

        }

        return lcPainter;
    });

    _canvas->deletePainterFunctor([&]
    (LcPainter * painter) {
        if (lcPainter != NULL) {
            delete painter;
            lcPainter = NULL;
        }
    });

    _canvas->newDeviceSize(800, 600);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset1(10.0, 10.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();

    builder->move(_offset1);
    builder->execute();

    _canvas->render([&](
                        LcPainter * lcPainter
    ) {
    }, [&](
        LcPainter * lcPainter
    ) {
    });

    _canvas->autoScale();

    lcPainter->writePNG("/tmp/out.png");

    return 0;
}


