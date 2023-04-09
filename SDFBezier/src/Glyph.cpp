#include <ft2build.h>
#include FT_FREETYPE_H

#include "Glyph.h"
#include "glm/geometric.hpp"

#include <chrono>
#include <iostream>

bool right(const glm::vec2& d, const glm::vec2& v)
{
    return (d.x * v.y - d.y * v.x) < 0 ? true : false;
}


Glyph::Glyph(const std::string& path, const char& character)
{
    m_Character = character;
	m_Path = path;

    loadGlyph();
}



inline float distanceSq(const glm::vec2& a, const glm::vec2& b)
{
    const glm::vec2 tmp = a - b;
    return glm::dot(tmp, tmp);
}

// TODO read correctly ttf
OutLines Glyph::readTTF()
{
    OutLines outLines;

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

    outLines.points.resize(faceOutline.n_points * 2);
    outLines.flags.resize(faceOutline.n_points);

    for (size_t i = 0; i < faceOutline.n_points; i++)
    {
        outLines.flags[i] = faceOutline.tags[i];
        outLines.points[2 * i] = (float)faceOutline.points[i].x / 64.0;
        outLines.points[2 * i + 1] = (float)faceOutline.points[i].y / 64.0;
    }

    outLines.contour.resize(faceOutline.n_contours);
    for (size_t i = 0; i < faceOutline.n_contours; i++)
        outLines.contour[i] = faceOutline.contours[i];

    return outLines;
}

void Glyph::BuildCurves(const OutLines& OutLines)
{
    const size_t pointsCount = OutLines.flags.size();
    
    uint16_t start = 0;
    for (const auto& end : OutLines.contour)
    {
        Bezier bezier{ {{OutLines.points[2 * start], OutLines.points[2 * start + 1]}} };
        size_t i = start;
        bool Sbreak = true;
        while (Sbreak)
        {
            i++;
            if (i > end)
            {
                i = start;
                Sbreak = false;
            }
            
            bezier.addPoint({ OutLines.points[2 * i], OutLines.points[2 * i + 1] });

            if (OutLines.flags[i] & FT_CURVE_TAG_ON && bezier.getSize() > 1)
            {
                m_Curves.push_back(bezier);
                bezier = Bezier{ {{ OutLines.points[2 * i], OutLines.points[2 * i + 1] }} };
            }
        }
        start = end + 1;
    }

    
}