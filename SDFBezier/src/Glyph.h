#pragma once

#include "Bezier.h"

#include <string>
#include <vector>

struct JSP
{
	std::vector<float> points;
	std::vector<uint8_t> flags;
	std::vector<uint16_t> contour;
};

class Glyph
{
public:
	char m_Character;
	std::vector<Bezier> m_Curves;
	std::string m_Path;

	Glyph(const std::string& path, const char& character);
	void loadGlyph() { BuildCurves(readTTF()); };

private:
	JSP readTTF();
	void BuildCurves(const JSP& jsp);
};