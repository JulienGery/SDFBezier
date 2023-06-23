#include <ft2build.h>
#include FT_FREETYPE_H

#include "Glyph.h"
#include "glm/geometric.hpp"
#include "glm/mat2x2.hpp"
#include "glm/trigonometric.hpp"

#include <chrono>
#include <complex>
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

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

    m_Outlines.points.resize(faceOutline.n_points * 2);
    m_Outlines.flags.resize(faceOutline.n_points);

    for (size_t i = 0; i < faceOutline.n_points; i++)
    {
        m_Outlines.flags[i] = faceOutline.tags[i];
        m_Outlines.points[2 * i] = (float)faceOutline.points[i].x / 64.0;
        m_Outlines.points[2 * i + 1] = (float)faceOutline.points[i].y / 64.0;
    }

    m_Outlines.contour.resize(faceOutline.n_contours);
    for (size_t i = 0; i < faceOutline.n_contours; i++)
        m_Outlines.contour[i] = faceOutline.contours[i];

}

void Glyph::BuildCurves()
{
    const size_t pointsCount = m_Outlines.flags.size();
    
    uint16_t start = 0;
    for (const auto& end : m_Outlines.contour)
    {
        Bezier bezier{ {{m_Outlines.points[2 * start], m_Outlines.points[2 * start + 1]}} };
        size_t i = start;
        bool keepgoing = true;
        glm::vec2 previusPoint{NAN, NAN};

        while (keepgoing)
        {
            i++;
            if (i > end)
            {
                i = start;
                keepgoing = false;
            }
            
            const glm::vec2 point{ m_Outlines.points[2 * i], m_Outlines.points[2 * i + 1] };

            //if (glm::distance(point, previusPoint) < 10)
            //{
            //    //std::cout << "continue\n";
            //    //continue;
            //}

            if (!(m_Outlines.flags[i] & FT_CURVE_TAG_ON) && bezier.size() == 2)
            {
                const glm::vec2 middle = (previusPoint + point) / 2.f;
                bezier.addPoint(middle);
                m_Curves.push_back(bezier);
                bezier = Bezier{ {middle, point} };
            }
            else if (m_Outlines.flags[i] & FT_CURVE_TAG_ON)
            {
                bezier.addPoint(point);
                m_Curves.push_back(bezier);
                bezier = Bezier{ {point} };
            }
            else
                bezier.addPoint(point);

            previusPoint = point;
        }
        m_Split.push_back(m_Curves.size() - 1);

        start = end + 1;
    }
}


glm::mat2 rotationMatrix(const float o)
{
    return {
        glm::cos(o), glm::sin(o),		// cos -sin
        -glm::sin(o),  glm::cos(o)		// sin  cos
    };
}

glm::vec2 bisector(const glm::vec2 a, const glm::vec2 b)
{
    const glm::vec2 x = glm::normalize(a);

    const std::complex<double> zx{ a.x, a.y };
    const std::complex<double> zy{ b.x, b.y };

    float o = std::arg(zy / zx) / 2.0;
    
    if (o > 0.)
        o -= M_PI;

    const auto matrix = rotationMatrix(o);

    return matrix * x;
}

void Glyph::generateBisectors()
{
    m_Bisectors.resize(m_Curves.size());

    uint16_t start = 0;
    for (const auto& end : m_Split)
    {
        glm::vec2 startBisector = bisector(
            -m_Curves[end].getDerivateEnd(), m_Curves[start].getStartDerivate()
        );

        for (size_t i = start; i < end; i++)
        {
            const glm::vec2 endBisector = bisector(
                -m_Curves[i].getDerivateEnd(), m_Curves[i + 1].getStartDerivate()
            );

            m_Bisectors[i] = { startBisector, endBisector };
            startBisector = endBisector;
        }

        const glm::vec2 endAngle = bisector(
            -m_Curves[end].getDerivateEnd(), m_Curves[start].getStartDerivate()
        );

        m_Bisectors[end] = { startBisector, endAngle };
        start = end + 1;
    }
}