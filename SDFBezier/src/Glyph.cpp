#include <ft2build.h>
#include FT_FREETYPE_H

#include "Glyph.h"
#include "glm/geometric.hpp"

#include <chrono>
#include <iostream>

Glyph::Glyph(const std::string& path, const char& character)
{
    m_Character = character;
	m_Path = path;

    loadGlyph();
}

bool inline pct(const glm::vec2& point, const Bezier& curve)
{
    const glm::vec2 _pct = glm::step(point, curve.getTopRight()) * glm::step(curve.getBottomLeft(), point);
    return _pct.y * _pct.x;
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

//Custom Glyph::findClosestPoint(const glm::vec2& point, double& start) const
//{
//    Custom PointAndDistance;
//    bool IsInsideBox = false;
//    size_t BoxIndex;
//
//    //check if point is inside a bounding box
//    for (size_t i = 0; i < m_Curves.size(); i++)
//    {
//        const auto& curve = m_Curves[i];
//        if (pct(point, curve))
//        {
//            IsInsideBox = true;
//            BoxIndex = i;
//            PointAndDistance = curve.findClosestPoint(point, start);
//            break;
//        }
//    }
//
//    if (IsInsideBox)
//    {
//        for (size_t i = 0; i < m_Curves.size(); i++)
//            if (i != BoxIndex)
//            {
//                const auto& curve = m_Curves[i];
//                if (pct(point, curve))
//                {
//                    const Custom test = curve.findClosestPoint(point, start);
//                    if (test.distance < PointAndDistance.distance)
//                        PointAndDistance = test;
//                }
//                else
//                {
//                    const glm::vec2 testPoint = curve.findClosestPointBoundingBox(point);
//                    const float distance = distanceSq(testPoint, point);
//                    if (distance < PointAndDistance.distance)
//                    {
//                        const Custom test = curve.findClosestPoint(point, start);
//                        if (test.distance < PointAndDistance.distance)
//                            PointAndDistance = test;
//                    }
//                }
//            }
//        return PointAndDistance;
//    }
//
//    // if the point is not inside a bounding box we search for the closest bounding box 
//    // which hold the closest point on a curve.
//    // since the distance from the point to the closest point on a curve is always higher than 
//    // the distance from the point to the closest point on a bouding box.
//
//    PointAndDistance.point = m_Curves[0].findClosestPointBoundingBox(point);
//    PointAndDistance.distance = distanceSq(point, PointAndDistance.point);
//
//    for (size_t i = 1; i < m_Curves.size(); i++)
//    {
//        const glm::vec2 testPoint = m_Curves[i].findClosestPointBoundingBox(point);
//        const float testDistance = distanceSq(point, testPoint);
//        if (testDistance < PointAndDistance.distance)
//        {
//            const Custom test = m_Curves[i].findClosestPoint(point, start);
//            if (test.distance < PointAndDistance.distance)
//                PointAndDistance = test;
//        }
//    }
//
//    return PointAndDistance;
//}


IndexRootDistancePoint Glyph::findClosestPoint(const glm::vec2& point, double& start) const
{
    size_t index = 0;
    RootDistancePoint DistanceAndPoint = m_Curves[0].findClosestPoint(point, start);
    for (size_t i = 1; i < m_Curves.size(); i++)
    {
        const RootDistancePoint test = m_Curves[i].findClosestPoint(point, start);
        if (test.distance < DistanceAndPoint.distance)
        {
            DistanceAndPoint = test;
            index = i;
        }
    }

    return { index, DistanceAndPoint.root, DistanceAndPoint.distance, DistanceAndPoint.point };
}