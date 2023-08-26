#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_GLYPH_H

#include "Glyph.h"

#include <stdexcept>

Glyph::Glyph(const std::string& path, const char& character)
{
    m_Character = character;
	m_Path = path;

    loadGlyph();
}

int TMPMoveTO(const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);
    glyph->m_Contours.push_back(Contour{});

    glyph->previusPoint = { to->x, to->y };
    return 0;
}

int TMPLineTO(const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);

    const size_t last = glyph->m_Contours.size() - 1;
    glyph->m_Contours[last].push_back(
        Bezier{ {glyph->previusPoint, {to->x, to->y} } }
    );

    glyph->previusPoint = { to->x, to->y };
    return 0;
}

int TMPConicTO(const FT_Vector* control, const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);

    const size_t last = glyph->m_Contours.size() - 1;
    glyph->m_Contours[last].push_back(
        Bezier{ {glyph->previusPoint , {control->x , control->y }, {to->x , to->y }} }
    );

    glyph->previusPoint = { to->x, to->y };
    return 0;
}

int TMPCubicTO( const FT_Vector* control1,
                const FT_Vector* control2,
                const FT_Vector* to,
                void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);

    const size_t last = glyph->m_Contours.size() - 1;
    glyph->m_Contours[last].push_back( Bezier{
        {glyph->previusPoint , {control1->x , control1->y },
        {control2->x , control2->y }, {to->x , to->y }} }
    );

    glyph->previusPoint = { to->x, to->y };
    return 0;
}


void Glyph::readTTF()
{
    FT_Library library;
    FT_Error error;
    FT_Face face;
    FT_UInt glyphIndex;

    error = FT_Init_FreeType(&library);
    if (error)
        throw std::runtime_error("failed to init FT");

    error = FT_New_Face(library, m_Path.c_str(), 0, &face);
    if (error)
        throw std::runtime_error("failed to read file");

    error = FT_Set_Char_Size(face, 0, 1, 0, 0);
    if (error)
        throw std::runtime_error("failed to set char size");

    glyphIndex = FT_Get_Char_Index(face, m_Character);
    if (!glyphIndex)
        throw std::runtime_error("failed to get index");

    error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_SCALE);
    if (error)
        throw std::runtime_error("failed to load glyph");

    FT_Outline_Funcs functions{};
    functions.move_to = TMPMoveTO;
    functions.line_to = TMPLineTO;
    functions.conic_to = TMPConicTO;
    functions.cubic_to = TMPCubicTO;
    functions.delta = 0;
    functions.shift = 0;

    error = FT_Outline_Decompose(&face->glyph->outline, &functions, this);

    if (FT_Outline_Get_Orientation(&face->glyph->outline) == FT_ORIENTATION_FILL_LEFT)
        reverse();

    error = FT_Done_FreeType(library);
    if (error)
        std::cout << "error when closing freetype\n";

}

void Glyph::preprocessing()
{
    for (size_t i = 0; i < m_Contours.size(); i++)
    {
        m_Contours[i].buildBisector();
        m_Contours[i].computeBbox();
        m_Contours[i].assignColors();
    }

    //TODO rework that part
    computeBbox();
    double pxRange = 2.0;
    double l = m_bbox.x, b = m_bbox.y, r = m_bbox.z, t = m_bbox.w;

    glm::vec2 frame{1920, 1080};
    double m = .5;
    frame -= 2.0 * m * pxRange;

    double scaleFactor;

    if(l >= r || b >= t)
        l = 0, b = 0, r = 1, t = 1;
    if(frame.x <= 0 || frame.y <= 0)
        throw std::runtime_error("cannot fit the specifield pixel range.");

    glm::vec2 dims(r - l, t - b);

    if(dims.x * frame.y < dims.y * frame.x)
    {
        // transate.set(.5*(frame.x/frame.y * dims.y - dims.x) - l, -b);
        scaleFactor = frame.y/dims.y;
    }
    else
    {
        // translate.set(-l, .5 * (frame.y/frame.y * dims.x - dims.y) - b);
        scaleFactor = frame.x / dims.x;
    }

    scaleFactor = 1./
        std::max(m_bbox.z - m_bbox.x, m_bbox.w - m_bbox.y);


    scale(scaleFactor);
}

void Glyph::TMPGETBEZIER()
{
    for (const auto& outline : m_Contours)
    {
        for (const auto& bisector : outline.m_Bisector)
            m_Bisectors.push_back(bisector);
        for (const auto& curve : outline.m_Curves)
            m_Curves.push_back(curve);
    }
}

void Glyph::reverse()
{
    for (size_t i = 0; i < m_Contours.size(); i++)
        m_Contours[i].reverse();
}

void Glyph::scale(const float c)
{
    for (size_t i = 0; i < m_Contours.size(); i++)
        m_Contours[i].scale(c);
    m_bbox *= c;
}

void Glyph::translate(const glm::vec2 v)
{
    for(size_t i = 0; i < m_Contours.size(); i++)
        m_Contours[i].translate(v);
    const glm::vec4 tmp{v, v};
    m_bbox += tmp;
}

//TODO rework that
void Glyph::computeBbox()
{
    glm::vec2 bottomLeft = { m_Contours[0].m_bbox.x , m_Contours[0].m_bbox.y };
    glm::vec2 topRight = { m_Contours[0].m_bbox.z , m_Contours[0].m_bbox.w };

    for (size_t i = 1; i < m_Contours.size(); i++)
    {
        const auto bbox = m_Contours[i].m_bbox;
        if (bbox.x < bottomLeft.x)
            bottomLeft.x = bbox.x;
        if (bbox.y < bottomLeft.y)
            bottomLeft.y = bbox.y;

        if (bbox.z > topRight.x)
            topRight.x = bbox.z;
        if (bbox.w > topRight.y)
            topRight.y = bbox.w;
    }

    m_bbox = { bottomLeft, topRight };
}
