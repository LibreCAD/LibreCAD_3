#include "gl_font.h"
#include <cstddef>
#include <string>
#include <vector>
using namespace lc::viewer::opengl;


namespace {

/// Decode UTF-8 into Unicode code points.
///
/// This replaces std::wstring_convert with std::codecvt_utf8_utf16, which
/// C++17 deprecates. It was also the wrong tool for the job three times over:
/// it yields UTF-16, so anything outside the basic multilingual plane arrived
/// as a surrogate pair and indexed the glyph map by half a character; it
/// throws std::range_error on malformed input, from inside a draw call; and
/// getTextExtend then cast each unit through `unsigned char`, truncating every
/// code point to its low eight bits, so every non-ASCII string was measured
/// against the wrong glyphs.
///
/// A malformed sequence becomes U+FFFD, which is what a text renderer wants:
/// the replacement glyph, not an exception. Overlong encodings, surrogate
/// halves and out-of-range values are malformed too -- they must not be
/// trusted as map indices just because they decoded arithmetically.
std::vector<unsigned int> decodeUtf8(const std::string& text) {
    constexpr unsigned int kReplacement = 0xFFFDu;
    std::vector<unsigned int> points;
    points.reserve(text.size());

    for (std::size_t i = 0; i < text.size();) {
        const unsigned char lead = static_cast<unsigned char>(text[i]);

        if (lead < 0x80u) {
            points.push_back(lead);
            ++i;
            continue;
        }

        unsigned int point = 0;
        std::size_t continuations = 0;
        unsigned int smallest = 0;

        if ((lead & 0xE0u) == 0xC0u) {
            point = lead & 0x1Fu;
            continuations = 1;
            smallest = 0x80u;
        } else if ((lead & 0xF0u) == 0xE0u) {
            point = lead & 0x0Fu;
            continuations = 2;
            smallest = 0x800u;
        } else if ((lead & 0xF8u) == 0xF0u) {
            point = lead & 0x07u;
            continuations = 3;
            smallest = 0x10000u;
        } else {
            // A continuation byte or a 5/6-byte lead: not a start of anything.
            points.push_back(kReplacement);
            ++i;
            continue;
        }

        bool complete = i + continuations < text.size();
        if (complete) {
            for (std::size_t k = 1; k <= continuations; k++) {
                const unsigned char next = static_cast<unsigned char>(text[i + k]);
                if ((next & 0xC0u) != 0x80u) {
                    complete = false;
                    break;
                }
                point = (point << 6) | (next & 0x3Fu);
            }
        }

        if (!complete || point < smallest || point > 0x10FFFFu
            || (point >= 0xD800u && point <= 0xDFFFu)) {
            points.push_back(kReplacement);
            // Advance one byte, not the whole sequence: the bytes after a bad
            // lead may themselves start a good one.
            ++i;
            continue;
        }

        points.push_back(point);
        i += continuations + 1;
    }

    return points;
}

}  // namespace

GL_Font::GL_Font()
{
}

GL_Font::~GL_Font()
{
}

bool GL_Font::readyFont(const std::string& path, std::string& fontFamily, std::string& fontStyle)
{
    const char* font_path= path.c_str();

    FT_Library ft;
    FT_Face face;

    if (FT_Init_FreeType(&ft))
    {
        //("ERROR::FREETYPE: Could not init FreeType Library");
        return false;
    }

    if (FT_New_Face(ft, font_path, 0, &face))
    {
        //("ERROR::FREETYPE: Failed to load font");
        return false;
    }

    FT_Set_Pixel_Sizes(face, 64,64);

    float xpos,ypos,w,h;

    //=========================================
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (unsigned int c = 0; c < 256; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            //("ERROR::FREETYTPE: Failed to load Glyph");
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // make texture of bitmap data
        glTexImage2D( GL_TEXTURE_2D,
                      0,
                      GL_RED,
                      face->glyph->bitmap.width,
                      face->glyph->bitmap.rows,
                      0,
                      GL_RED,
                      GL_UNSIGNED_BYTE,
                      face->glyph->bitmap.buffer);

        glGenerateMipmap(GL_TEXTURE_2D);

        //Now saving coordinate data for the glypph

        xpos = (float)(face->glyph->bitmap_left);
        ypos = (float)(face->glyph->bitmap_top ) - (float)(face->glyph->bitmap.rows);

        w = face->glyph->bitmap.width;
        h = face->glyph->bitmap.rows;

        float _data[6*5]= {
            xpos,     -(ypos + h), 0.0f,   0.0, 0.0,
            xpos,     -(ypos),     0.0f,   0.0, 1.0,
            xpos + w, -(ypos),     0.0f,   1.0, 1.0,

            xpos,     -(ypos + h), 0.0f,   0.0, 0.0,
            xpos + w, -(ypos),     0.0f,   1.0, 1.0,
            xpos + w, -(ypos + h), 0.0f,   1.0, 0.0
        };

        //Now Pass the glyphs coordinate data to GPU
        GLuint vbo;
        GLuint vao;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, (sizeof(GLfloat) * 6 * 5), &_data[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)( 3 * sizeof(GLfloat))) ;

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        //Finally Save the Character Data

        Character ch = { texture, vbo,vao,
                         face->glyph->bitmap_left,
                         face->glyph->bitmap_top,
                         static_cast<int>(face->glyph->bitmap.width),
                         static_cast<int>(face->glyph->bitmap.rows),
                         static_cast<int>(face->glyph->advance.x),
                         static_cast<int>(face->glyph->advance.y)
                       };

        _characters.insert(std::pair<unsigned int, Character>(c, ch));
    }

    fontFamily = face->family_name;
    fontStyle = face->style_name;

    //Finished working with Freetype
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return true;
}

void GL_Font::renderText(std::string text,glm::mat4 proj,glm::mat4 view,glm::mat4 model,Shader* text_shader)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    std::map<unsigned int, Character>::iterator it;

    text_shader->bind();
    text_shader->setUniform1i("u_Texture",0);  // same slot of texture (optional)

    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    for (const unsigned int point : decodeUtf8(text))
    {
        it=_characters.find(point);

        if(it==_characters.end())
            it=_characters.find(' ');

        // The space fallback is itself only a guess: a font loaded without one
        // leaves nothing to draw, and dereferencing end() here would be a
        // crash inside a draw call.
        if(it==_characters.end())
            continue;

        //compute the MVP matrix ( received -V ,-P  .. already have -M)
        glm::mat4 mvp=proj * view * model;

        text_shader->setUniformMat4f("u_MVP",mvp);  // Set MVP

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ((it->second)).textureID);

        // Bind the VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, ((it->second)).vbo);

        // Bind the VAO memory
        glBindVertexArray(((it->second)).vao);

        // Finally Render
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Unbind
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        model=glm::translate(model,glm::vec3( ( (it->second).x_advance >> 6 ),0.0,0.0));
    }

    text_shader->unbind();
}

GL_Text_Extend GL_Font::getTextExtend(std::string text, int font_size)
{
    std::map<unsigned int, Character>::iterator it;

    const std::vector<unsigned int> points = decodeUtf8(text);

    int total_adv_x=0;
    int total_adv_y=0;
    int bear_x=0;
    int bear_y=0;
    int height=0;
    int width=0;

    // The measurement used to prime itself from the first character before
    // checking that the font had one, so an empty string dereferenced end()
    // and so did any string starting with an unmapped character. The loop
    // below seeds itself from the first glyph it actually finds instead.
    bool seeded=false;
    int max_y=0;

    for (const unsigned int point : points)
    {
        it=_characters.find(point);

        if(it==_characters.end())
            it=_characters.find(' ');

        if(it==_characters.end())
            continue;

        if(!seeded)
        {
            bear_x=((it->second).x_bearing);
            bear_y=((it->second).y_bearing);
            max_y=((it->second).y_bearing)+((it->second).height);
            seeded=true;
        }

        total_adv_x =total_adv_x + ((it->second).x_advance >> 6 );

        if( ((it->second).y_bearing) < bear_y )
            bear_y=((it->second).y_bearing);

        if( ( ((it->second).y_bearing)+((it->second).height) ) > max_y )
            max_y=( ((it->second).y_bearing)+((it->second).height) );
    }

    height=max_y - bear_y;
    width=total_adv_x;

    total_adv_x *=(font_size/64.0);
    total_adv_y*=(font_size/64.0);
    bear_x*=(font_size/64.0);
    bear_y*=(font_size/64.0);
    height*=(font_size/64.0);
    width*=(font_size/64.0);

    GL_Text_Extend TE= { bear_x,
                         bear_y,
                         width,
                         height,
                         total_adv_x,
                         total_adv_y
                       };

    return TE;
}
