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
    //glyph->m_Outlines.push_back()


    glyph->previusPoint = { to->x, to->y };
    return 0;
}

int TMPLineTO(const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);
    glyph->m_Curves.push_back(
        Bezier{ {glyph->previusPoint / 64.f, {to->x / 64.0, to->y / 64.0}} }
    );

    glyph->previusPoint = { to->x, to->y };
    return 0;
}

int TMPConicTO(const FT_Vector* control, const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);
    glyph->m_Curves.push_back(
        Bezier{ {glyph->previusPoint / 64.f, {control->x / 64.0, control->y / 64.0}, {to->x / 64.0, to->y / 64.0}} }
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
    glyph->m_Curves.push_back(
        Bezier{
            {glyph->previusPoint / 64.f, {control1->x / 64.f, control1->y / 64.f},
            {control2->x / 64.f, control2->y / 64.f}, {to->x / 64.f, to->y / 64.f}
            }}
    );


    glyph->previusPoint = { to->x, to->y };
    return 0;
}

// TODO read correctly ttf
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

    error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP);
    if (error)
        throw std::runtime_error("failed to load glyph");

    //FT_Outline_Funcs function{};
    //function.move_to = TMPMoveTO;
    //function.line_to = TMPLineTO;
    //function.conic_to = TMPConicTO;
    //function.cubic_to = TMPCubicTO;
    //function.delta = 0;
    //function.shift = 0;

    //error = FT_Outline_Decompose(&face->glyph->outline, &function, this);
    
    const auto bbox = face->bbox;

    std::cout << bbox.xMin / 64.0 << ' ' << bbox.xMax / 64.0 << '\t' << bbox.yMin / 64.0 << ' ' << bbox.yMax / 64.0 << '\n';

    FT_Outline_Reverse(&face->glyph->outline);


    FT_Outline faceOutline = face->glyph->outline;

    glyphData.points.resize(faceOutline.n_points);
    glyphData.flags.resize(faceOutline.n_points);

    for (size_t i = 0; i < faceOutline.n_points; i++)
    {
        glyphData.flags[i] = faceOutline.tags[i];
        glyphData.points[i].x = (float)faceOutline.points[i].x / 64.0;
        glyphData.points[i].y = (float)faceOutline.points[i].y / 64.0;
    }

    glyphData.contour.resize(faceOutline.n_contours);
    for (size_t i = 0; i < faceOutline.n_contours; i++)
        glyphData.contour[i] = faceOutline.contours[i];

    error = FT_Done_FreeType(library);
    if (error)
        std::cout << "error when closing freetype\n";
}

void Glyph::buildOutlines()
{
    uint16_t start = 0;
    
    for (size_t i = 0; i < glyphData.contour.size(); i++)
    {
        const uint16_t end = glyphData.contour[i];
        const std::vector<glm::vec2> points{&glyphData.points[start], &glyphData.points[end + 1]};
        const std::vector<uint16_t> flags{&glyphData.flags[start], &glyphData.flags[end + 1]};
        const Contour outline{ points, flags };
        m_Outlines.push_back(outline);

        start = end + 1;
    }
}

void Glyph::TMPGETBEZIER()
{
    for (const auto& outline : m_Outlines)
    {
        for (const auto& bisector : outline.m_Bisector)
            m_Bisectors.push_back(bisector);
        for (const auto& curve : outline.m_Curves)
            m_Curves.push_back(curve);
    }
}
