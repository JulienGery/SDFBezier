#include <ft2build.h>
#include FT_FREETYPE_H

#include "Glyph.h"

#include <stdexcept>

Glyph::Glyph(const std::string& path, const char& character)
{
    m_Character = character;
	m_Path = path;

    loadGlyph();
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

}

void Glyph::buildOutlines()
{
    uint16_t start = 0;

    for (size_t i = 0; i < glyphData.contour.size(); i++)
    {
        const uint16_t end = glyphData.contour[i];
        const std::vector<glm::vec2> points{&glyphData.points[start], &glyphData.points[end + 1]};
        const std::vector<uint16_t> flags{&glyphData.flags[start], &glyphData.flags[end + 1]};
        const Outline outline{ points, flags };
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
