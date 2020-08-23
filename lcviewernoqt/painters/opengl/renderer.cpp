#include "renderer.h"
using namespace lc::viewer::opengl;

Renderer::Renderer()
{
    _ctm=glm::mat4(1.0f);
    _view=_ctm;
    _shader_path=(lc::viewer::viewerSettings.get(SETTINGS_GL_SHADER_PATH)->getString());
    _font_path=(lc::viewer::viewerSettings.get(SETTINGS_GL_FONT_PATH)->getString());
}

Renderer::~Renderer()
{
}

void Renderer::createResources()
{
    _shaders.basic_shader = new Shader();
    _shaders.basic_shader->gen(_shader_path+"basic_shader.shader");
    _shaders.basic_shader->unbind();

    _shaders.gradient_shader = new Shader();
    _shaders.gradient_shader->gen(_shader_path+"color_vertex_shader.shader", [](GLuint programId) {
        glBindAttribLocation(programId, 0, "pos");
        glBindAttribLocation(programId, 1, "prev_distance");
        glBindAttribLocation(programId, 2, "col");
    });
    _shaders.gradient_shader->unbind();

    _shaders.thickline_shader = new Shader();
    _shaders.thickline_shader->gen(_shader_path+"thickline_shader.shader");
    _shaders.thickline_shader->unbind();

    _shaders.linepattern_shader = new Shader();
    _shaders.linepattern_shader->gen(_shader_path+"dash_pattern_shader.shader");
    _shaders.linepattern_shader->unbind();

    _shaders.text_shader = new Shader();
    _shaders.text_shader->gen(_shader_path+"text_shader.shader");
    _shaders.text_shader->unbind();

    _cacherPtr->setShaderBook(_shaders);

    _fonts.createDefaultFont("ABeeZee-Regular",_font_path+"ABeeZee-Regular.otf");
    _fonts.createFontsFromDir(_font_path);

    _cacherPtr->setFontBook(_fonts);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::setCacherRef(Cacher* ch)
{
    _cacherPtr=ch;
}
//-------------------------------------------------
void Renderer::updateProjection(float l,float r,float b,float t)
{
    _projB=glm::ortho(-r/2,r/2,b/2,-b/2,-1.0f,1.0f);
    _proj=glm::ortho(l,r,b,t,-1.0f,1.0f);

    if(_shaders.thickline_shader!=NULL)
    {
        _shaders.thickline_shader->bind();
        _shaders.thickline_shader->setUniform2f("WIN_SCALE",r,b);
        _shaders.thickline_shader->unbind();
    }

    if(_shaders.linepattern_shader!=NULL)
    {
        _shaders.linepattern_shader->bind();
        _shaders.linepattern_shader->setUniform2f("WIN_SCALE",r,b);
        _shaders.linepattern_shader->unbind();
    }
}

void Renderer::updateView()
{
    _view=_ctm;
}
//------------------------
void Renderer::updateScale(float scale_f)
{
    _ctm=glm::scale(_ctm,glm::vec3(scale_f,scale_f,scale_f));
    _view=_ctm;
}

void Renderer::updateTranslate(float x,float y)
{
    _ctm=glm::translate(_ctm,glm::vec3(x,y,0.0));
    _view=_ctm;
}

void Renderer::updateRotate(float angle)
{
    _ctm=glm::rotate(_ctm, angle, glm::vec3(0.0f,0.0f,1.0f) );
    _view=_ctm;
}

void Renderer::resetTransformations()
{
    _ctm=glm::mat4(1.0f);
    _view=_ctm;
}

double Renderer::getScale()
{
    return _ctm[2][2];
}

double Renderer::getTranslateX()
{
    return _ctm[3][0];
}

double Renderer::getTranslateY()
{
    return _ctm[3][1];
}

//-----------------------

void Renderer::deviceToUser(double* x, double* y)
{
    glm::vec4 temp=glm::vec4(*x,*y,0,1);
    temp=glm::inverse(_ctm) * temp;
    *x=temp.x;
    *y=temp.y;
}

void Renderer::deviceToUserDistance(double* x, double* y)
{
    glm::vec4 temp_vec=glm::vec4(*x,*y,0,1);
    glm::mat4 temp_mat=_ctm;
    temp_mat[3][0]=0;
    temp_mat[3][1]=0;
    temp_vec=glm::inverse( temp_mat ) * temp_vec;
    *x=temp_vec.x;
    *y=temp_vec.y;
}

void Renderer::userToDevice(double* x, double* y)
{
    glm::vec4 temp=glm::vec4(*x,*y,0,1);
    temp=glm::mat4(_ctm) * temp;
    *x=temp.x;
    *y=temp.y;
}

void Renderer::userToDeviceDistance(double* x, double* y)
{
    glm::vec4 temp_vec=glm::vec4(*x,*y,0,1);
    glm::mat4 temp_mat=_ctm;
    temp_mat[3][0]=0;
    temp_mat[3][1]=0;
    temp_vec=glm::inverse( temp_mat ) * temp_vec;
    *x=temp_vec.x;
    *y=temp_vec.y;
}

//------------------------------------------

void Renderer::save()
{
    context_att current_context;
    current_context.ctm = this->_ctm;
    _context_stack.push(current_context);
}

void Renderer::restore()
{
    context_att prev_context = _context_stack.top();
    this->_ctm = prev_context.ctm;
    _context_stack.pop();
    _view=_ctm;
}
//----------------------------------------------------
void Renderer::readyCurrentEntity()
{
    addDataToCurrentEntity();
    getCurrentEntity()->setType(_shaders);
    getCurrentEntity()->setFont(_fonts,_font_style);
}

void Renderer::selectColor(float R,float G,float B,float A)
{
    _shaders.basic_shader->bind();
    _shaders.basic_shader->setUniform4f("u_Color",R,G,B,A);
    _shaders.basic_shader->unbind();

    _shaders.thickline_shader->bind();
    _shaders.thickline_shader->setUniform4f("u_Color",R,G,B,A);
    _shaders.thickline_shader->unbind();

    _shaders.linepattern_shader->bind();
    _shaders.linepattern_shader->setUniform4f("u_Color",R,G,B,A);
    _shaders.linepattern_shader->unbind();

    _shaders.text_shader->bind();
    _shaders.text_shader->setUniform4f("u_Color",R,G,B,A);
    _shaders.text_shader->unbind();
}

GL_Text_Extend Renderer::getTextExtend(const char* text_val)
{
    GL_Text_Extend TE=(_fonts.pickFont(_font_style))->getTextExtend(std::string(text_val),_text_height);
    return TE;
}

void Renderer::render()
{
    //load data to current entity
    readyCurrentEntity();
    // Send the _proj & _view matrix needed to draw
    getCurrentEntity()->draw(_proj,_projB,_view);
    //Free the GPU memory
    getCurrentEntity()->freeGPU();
    //Clear data in buffer(CPU)
    setDefault();
    //Adding a new entity( Shape_entity )
    deleteCurrentEntity();
    setNewShapeEntity();
}

void Renderer::renderCachedEntity(GL_Entity* cached_entity)
{
    getCurrentEntity()->unbind();
    save();
    cached_entity->draw(_proj,_projB,_view);
    restore();
}

void Renderer::renderCachedPack(GL_Pack* pack)
{
    int l=pack->packSize();
    GL_Entity* gl_entity_in_pack;

    for(int i=0; i<l; i++)
    {
        gl_entity_in_pack=pack->getEntityAt(i);
        renderCachedEntity(gl_entity_in_pack);
    }
}

const Font_Book& Renderer::fontBook() const
{
    return _fonts;
}

Font_Book& Renderer::fontBook() {
    return _fonts;
}
