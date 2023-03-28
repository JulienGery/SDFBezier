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

bool inline pct(const glm::vec2& point, const Bezier& curve)
{
    const glm::vec2& topRight = curve.getTopRight();
    const glm::vec2& botomLeft = curve.getBottomLeft();
    return point.x > botomLeft.x && point.y > botomLeft.y && point.x < topRight.x && point.y < topRight.y;
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
        Bezier bezier{ {OutLines.points[2 * start], OutLines.points[2 * start + 1]}, 100 };
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

            if (OutLines.flags[i] & FT_CURVE_TAG_ON && bezier.getPointCount() > 1)
            {
                m_Curves.push_back(bezier);
                bezier = Bezier{ { OutLines.points[2 * i], OutLines.points[2 * i + 1] } , 10000 };
            }
        }
        start = end + 1;
    }

    for (auto& i : m_Curves)
        i.updateExtremum();

    m_TopRight = m_Curves[0].getTopRight();
    m_BottomLeft = m_Curves[0].getBottomLeft();
    for (const auto& i : m_Curves)
    {
        const glm::vec2 topRight = i.getTopRight();
        const glm::vec2 bottomLeft = i.getBottomLeft();

        if (topRight.x > m_TopRight.x)
            m_TopRight.x = topRight.x;
        if (topRight.y > m_TopRight.y)
            m_TopRight.y = topRight.y;
        if (bottomLeft.x < m_BottomLeft.x)
            m_BottomLeft.x = bottomLeft.x;
        if (bottomLeft.y < m_BottomLeft.y)
            m_BottomLeft.y = bottomLeft.y;
    }
}

void Glyph::computeBaryCentre()
{
    for (auto& i : m_Curves)
        i.computeBarycentre();

    m_Barycentre = m_Curves[0].getBarycentre();
    for (size_t i = 1; i < m_Curves.size(); i++)
        m_Barycentre += m_Curves[i].getBarycentre();

    m_Barycentre /= m_Curves.size();
}

// too messy
bool Glyph::inside(const glm::vec2& point, double& start) const
{
    bool isInside = false;
    bool isinsideBox = false;
    size_t StartingGlyphIndex = 0;
    RootDistancePoint rootDistancePoint;
    size_t index = 0;

    for (size_t i = 0; i < m_Curves.size(); i++)
    {
        const Bezier& curve = m_Curves[i];
        if (pct(point, curve))
        {
            isinsideBox = true;
            rootDistancePoint = curve.findClosestPoint(point, start);
            isInside = right(curve.derivate(rootDistancePoint.root), point - rootDistancePoint.point);
            StartingGlyphIndex = i;
            index = i;
            break;
        }
        else if (curve.getPointCount() == 2)
            StartingGlyphIndex = i;
    }

    if (!isinsideBox)
    {
        rootDistancePoint = m_Curves[StartingGlyphIndex].findClosestPoint(point, start);
        isInside = right(m_Curves[StartingGlyphIndex].derivate(rootDistancePoint.root), point - rootDistancePoint.point);
        index = StartingGlyphIndex;
    }

    for (size_t i = 0; i < m_Curves.size(); i++)
    {
        if (i == StartingGlyphIndex) continue;

        const Bezier& curve = m_Curves[i];
        RootDistancePoint test;

        if (!pct(point, curve))
        {
            test.point = curve.findClosestPointBoundingBox(point);
            test.distance = distanceSq(point, test.point);
            if (test.distance > rootDistancePoint.distance) 
                continue;
        }
        
        test = curve.findClosestPoint(point, start);
        if (test.distance < rootDistancePoint.distance)
        {
            rootDistancePoint = test;
            isInside = right(m_Curves[i].derivate(test.root), point - test.point);
            index = i;

        }
        else if (test.distance == rootDistancePoint.distance && !isInside)
        {
            isInside = right(m_Curves[i].derivate(test.root), point - test.point);
            rootDistancePoint = test;
            index = i;
        }
    }

    return isInside;
}


IndexRootDistancePoint Glyph::findClosestPoint(const glm::vec2& point, double& start) const
{
    size_t index = 0;
    bool isInside = false;
    RootDistancePoint DistanceAndPoint = m_Curves[0].findClosestPoint(point, start);
    for (size_t i = 1; i < m_Curves.size(); i++)
    {
        const RootDistancePoint test = m_Curves[i].findClosestPoint(point, start);
        if (test.distance < DistanceAndPoint.distance)
        {
            DistanceAndPoint = test;
            index = i;
            isInside = right(m_Curves[i].derivate(test.root), point - test.point);
        }
        else if (test.distance == DistanceAndPoint.distance)
        {
            if (!isInside && right(m_Curves[i].derivate(test.root), point - test.point))
            {
                DistanceAndPoint = test;
                index = i;
                isInside = true;
            }
        }
    }

    return { index, DistanceAndPoint.root, DistanceAndPoint.distance, DistanceAndPoint.point };
}
