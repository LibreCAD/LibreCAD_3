#include "openglcacherpainter.h"

OpenglCacherPainter::OpenglCacherPainter()
{
    _cacher= new Cacher();
    set_manager(_cacher);
}

void OpenglCacherPainter::create_resources()
{
    // NO Need ( main painter uses for rendering)
}

void OpenglCacherPainter::new_device_size(unsigned int width, unsigned int height)
{
    // NO Need ( main painter uses for rendering)
}


void OpenglCacherPainter::clear(double r, double g, double b)
{
    // NO Need ( main painter uses for rendering)
}

void OpenglCacherPainter::clear(double r, double g, double b, double a)
{
    // NO Need ( main painter uses for rendering)
}

double OpenglCacherPainter::scale()
{
    return _cacher->getScale();
}

void OpenglCacherPainter::scale(double s)
{
    _cacher->updateScale(s);
}

void OpenglCacherPainter::rotate(double r)
{
    _cacher->updateRotate(r);
}

void OpenglCacherPainter::stroke()
{
    _cacher->readyForNextEntity();
}

void OpenglCacherPainter::source_rgb(double r, double g, double b)
{
    // Implement later when required
}

void OpenglCacherPainter::source_rgba(double r, double g, double b, double a)
{
    // Implement later when required
}

void OpenglCacherPainter::translate(double x, double y)
{
    _cacher->updateTranslate(x,y );
}

void OpenglCacherPainter::user_to_device(double* x, double* y)
{
    // NO Need ( main painter uses for rendering)
}

void OpenglCacherPainter::device_to_user(double* x, double* y)
{
    // NO Need ( main painter uses for rendering)
}

void OpenglCacherPainter::user_to_device_distance(double* dx, double* dy)
{
    // NO Need ( main painter uses for rendering)
}

void OpenglCacherPainter::device_to_user_distance(double* dx, double* dy)
{
    // NO Need ( main painter uses for rendering)
}

void OpenglCacherPainter::text(const char* text_val)
{
    _cacher->setNewTextEntity();
    _cacher->selectFontValue(text_val);
}

TextExtends OpenglCacherPainter::text_extends(const char* text_val)
{
    TextExtends te;
    GL_Text_Extend extents=_cacher->getTextExtend(text_val);

    te.height = extents.height;
    te.width = extents.width;
    te.x_advance = extents.x_advance;
    te.x_bearing = extents.x_bearing;
    te.y_advance = extents.y_advance;
    te.y_bearing = extents.y_bearing;

    return te;
}

void OpenglCacherPainter::save()
{
    //No NEED
}

void OpenglCacherPainter::restore()
{
    //No NEED
}

long OpenglCacherPainter::pattern_create_linear(double x1, double y1, double x2, double y2)
{
    _cacher->setNewGradientEntity();
    _cacher->addLinearGradient(x1,y1,x2,y2);
    return 0;
}

void OpenglCacherPainter::pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a)
{
    _cacher->addGradientColorPoint(r,g,b,a);
}

void OpenglCacherPainter::set_pattern_source(long pat)
{
}

void OpenglCacherPainter::pattern_destroy(long pat)
{
}

void OpenglCacherPainter::reset_transformations()
{
}

unsigned char* OpenglCacherPainter::data()
{
    return NULL;
}

long OpenglCacherPainter::image_create(const std::string& file)
{
    return 0;
}

void OpenglCacherPainter::image_destroy(long image)
{
}

void OpenglCacherPainter::image(long image, double uvx, double vy, double vvx, double vvy, double x, double y)
{
}

void OpenglCacherPainter::disable_antialias()
{
}

void OpenglCacherPainter::enable_antialias()
{
}

void OpenglCacherPainter::getTranslate(double* x, double* y)
{
}

bool OpenglCacherPainter::isCachingEnabled()
{
    return true;
}

void OpenglCacherPainter::startcaching()
{
    _cacher->readyFreshPack();
}

void OpenglCacherPainter::finishcaching(unsigned long id)
{
    _cacher->savePack(id);
}

LcPainter* OpenglCacherPainter::getCacherpainter()
{
    // NO Need ( main painter uses)
    return NULL;
}

bool OpenglCacherPainter::isEntityCached(unsigned long id)
{
    return _cacher->isPackCached(id);
}

void OpenglCacherPainter::renderEntityCached(unsigned long id)
{
    // No Need ( cant do rendering here)
}

void OpenglCacherPainter::deleteEntityCached(unsigned long id)
{
    _cacher->erasePack(id);
}
