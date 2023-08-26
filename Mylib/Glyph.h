#pragma once

#include "Contour.h"

#include <string>
#include <vector>


class Glyph
{
public:
	std::vector<Bezier> m_Curves; // TMP
	std::vector<glm::vec4> m_Bisectors; // TMP
	std::vector<Contour> m_Contours; // TMP
	glm::vec2 previusPoint; // TMP
	glm::vec4 m_bbox{0.}; //TMP should be private

	char m_Character;
	std::string m_Path;

	Glyph(const std::string& path, const char& character);

private:
	void readTTF();
	void TMPGETBEZIER();
	void loadGlyph() { readTTF(); preprocessing(); TMPGETBEZIER();  };
	void reverse();
	void scale(const float c);
    void translate(const glm::vec2 v);
	void computeBbox();
    void preprocessing();
};
