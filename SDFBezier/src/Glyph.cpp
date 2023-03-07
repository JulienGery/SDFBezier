//#include <ft2build.h>
//#include FT_FREETYPE_H
#define FT_CURVE_TAG_ON 0x01

#include "Glyph.h"

#include <iostream>
#include <fstream>

template<class T>
std::vector<T> readFile(const std::string& path) {
    std::ifstream file{ path, std::ios::binary | std::ios::ate };

    const size_t size = file.tellg();
    std::vector<T> buffer(size / sizeof(T));
    file.seekg(0);
    file.read((char*)buffer.data(), size);
    file.close();
    return buffer;
}

Glyph::Glyph(const std::string& path, const char& character)
{
    m_Character = character;
	m_Path = path;

    loadGlyph();
}

// TODO add real TTF read
JSP Glyph::readTTF()
{
    JSP jsp;
    jsp.points = readFile<float>("/Users/pincable/source/repos/tmp/points.bin");
    jsp.flags = readFile<uint8_t>("/Users/pincable/source/repos/tmp/on.bin");
    jsp.contour = readFile<uint16_t>("/Users/pincable/source/repos/tmp/contour.bin");

    return jsp;
}

void Glyph::BuildCurves(const JSP& jsp)
{
    const size_t pointsCount = jsp.flags.size();
    
    uint16_t start = 0;
    for (const auto& end : jsp.contour)
    {
        Bezier bezier{ {jsp.points[2 * start], jsp.points[2 * start + 1]}, 100 };
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

            bezier.addPoint({ jsp.points[2 * i], jsp.points[2 * i + 1] });

            if (jsp.flags[i] & FT_CURVE_TAG_ON && bezier.getPointCount() > 1)
            {
                m_Curves.push_back(bezier);
                bezier = Bezier{ { jsp.points[2 * i], jsp.points[2 * i + 1] } , 10000 };
            }
        }
        start = end + 1;
    }

    for (auto& i : m_Curves)
        i.updateExtremum();
}