#include "openglrenderpainter.h"

OpenglRenderPainter::OpenglRenderPainter(unsigned int width, unsigned int height)
{
    _renderer= new Renderer();
    set_manager(_renderer);
    new_device_size(width,height);

    _cacher_painter=new OpenglCacherPainter();
}

void OpenglRenderPainter::create_resources()
{
    //On Windows, GLEW context is not shared between libraries. We have to recreate it
    GLenum err = glewInit();

    if (err != GLEW_OK) {
        LOG_ERROR << "GLEW Error in lcviewernoqt: " << glewGetErrorString(err) << std::endl;
    }
    if (!GLEW_VERSION_2_1) {
        LOG_ERROR << "OpenGL version 2.1 is not available" << std::endl;
    }

    OpenglCacherPainter* cp= dynamic_cast<OpenglCacherPainter*>(_cacher_painter);
    _renderer->setCacherRef((((*cp)._cacher)));
    _renderer->createResources();
    _renderer->deleteCurrentEntity();
    _renderer->setNewShapeEntity();
}

void OpenglRenderPainter::new_device_size(unsigned int width, unsigned int height)
{
    if(height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    _device_width=(float)width;
    _device_height=(float)height;

    _renderer->updateProjection(0, _device_width, _device_height,0);
}

void OpenglRenderPainter::clear(double r, double g, double b)
{
    glClearColor(r,g,b,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenglRenderPainter::clear(double r, double g, double b, double a)
{
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


double OpenglRenderPainter::scale()
{
    return _renderer->getScale();
}

void OpenglRenderPainter::scale(double s)
{
    _renderer->updateScale(s);
}

void OpenglRenderPainter::rotate(double r)
{
    _renderer->updateRotate(r);
}

void OpenglRenderPainter::stroke()
{
    _renderer->render();
}

void OpenglRenderPainter::source_rgb(double r, double g, double b)
{
    _renderer->selectColor(r,g,b,1.0f);
}

void OpenglRenderPainter::source_rgba(double r, double g, double b, double a)
{
    _renderer->selectColor(r,g,b,a);
}

void OpenglRenderPainter::translate(double x, double y)
{
    _renderer->updateTranslate(x,y);
}

void OpenglRenderPainter::user_to_device(double* x, double* y)
{
    *y=-*y;
    _renderer->userToDevice(x,y);
}

void OpenglRenderPainter::device_to_user(double* x, double* y)
{
    _renderer->deviceToUser(x,y);
    *y=-*y;
}

void OpenglRenderPainter::user_to_device_distance(double* dx, double* dy)
{
    *dy = -*dy;
    _renderer->userToDeviceDistance(dx,dy);
}

void OpenglRenderPainter::device_to_user_distance(double* dx, double* dy)
{
    _renderer->deviceToUserDistance(dx,dy);
    *dy = -*dy;
}

void OpenglRenderPainter::text(const char* text_val)
{
    _renderer->deleteCurrentEntity();
    _renderer->setNewTextEntity();
    _renderer->selectFontValue(text_val);
}

TextExtends OpenglRenderPainter::text_extends(const char* text_val)
{
    TextExtends te;
    GL_Text_Extend extents=_renderer->getTextExtend(text_val);

    te.height = extents.height;
    te.width = extents.width;
    te.x_advance = extents.x_advance;
    te.x_bearing = extents.x_bearing;
    te.y_advance = extents.y_advance;
    te.y_bearing = extents.y_bearing;

    return te;
}

void OpenglRenderPainter::save()
{
    _renderer->save();
}

void OpenglRenderPainter::restore()
{
    _renderer->restore();
}

long OpenglRenderPainter::pattern_create_linear(double x1, double y1, double x2, double y2)
{
    _renderer->deleteCurrentEntity();
    _renderer->setNewGradientEntity();
    _renderer->addLinearGradient(x1,y1,x2,y2);
    return 0;
}

void OpenglRenderPainter::pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a)
{
    _renderer->addGradientColorPoint(r,g,b,a);
}

void OpenglRenderPainter::set_pattern_source(long pat)
{
}

void OpenglRenderPainter::pattern_destroy(long pat)
{
}

void OpenglRenderPainter::reset_transformations()
{
    _renderer->resetTransformations();
}

unsigned char* OpenglRenderPainter::data()
{
    return NULL;
}

long OpenglRenderPainter::image_create(const std::string& file)
{
    return 0;
}

void OpenglRenderPainter::image_destroy(long image)
{
}

void OpenglRenderPainter::image(long image, double uvx, double vy, double vvx, double vvy, double x, double y)
{
}

void OpenglRenderPainter::disable_antialias()
{
}

void OpenglRenderPainter::enable_antialias()
{
}

void OpenglRenderPainter::getTranslate(double* x, double* y)
{
}

bool OpenglRenderPainter::isCachingEnabled()
{
    return true;
}

void OpenglRenderPainter::startcaching()
{
    // NOTHING to DO.. (CachePainter Use this)
}

void OpenglRenderPainter::finishcaching(unsigned long id)
{
    // NOTHING to DO.. (CachePainter Use this)
}

LcPainter* OpenglRenderPainter::getCacherpainter()
{
    return _cacher_painter;
}

bool OpenglRenderPainter::isEntityCached(unsigned long id)
{
    return _cacher_painter->isEntityCached(id);
}

void OpenglRenderPainter::renderEntityCached(unsigned long id)
{
    OpenglCacherPainter* cp= dynamic_cast<OpenglCacherPainter*>(_cacher_painter);
    GL_Pack* _gl_pack=((*cp)._cacher)->getCachedPack(id);
    _renderer->renderCachedPack(_gl_pack);
}

void OpenglRenderPainter::deleteEntityCached(unsigned long id)
{
    _cacher_painter->deleteEntityCached(id);
}

std::vector<std::string> OpenglRenderPainter::getFontList() const {
    return _renderer->fontBook().getFontList();
}

void OpenglRenderPainter::addFontsFromPath(const std::vector<std::string>& paths) {
    Font_Book& fontBook = _renderer->fontBook();

    for (const std::string& path : paths) {
        fontBook.createFontsFromDir(path);
    }
}
