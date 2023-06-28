#pragma once

#include "Outline.h"

#include <string>
#include <vector>


struct GlyphData
{
	std::vector<glm::vec2> points;
	std::vector<uint8_t> flags;
	std::vector<uint16_t> contour; // contour
};



class Glyph
{
public:
	std::vector<Bezier> m_Curves; // TMP
	std::vector<glm::vec4> m_Bisectors; // TMP
	std::vector<Contour> m_Outlines; // TMP
	glm::vec2 previusPoint; // TMP
	
	char m_Character;
	std::string m_Path;


	Glyph(const std::string& path, const char& character);

private:
	GlyphData glyphData;

	void readTTF();
	void buildOutlines();
	void TMPGETBEZIER();
	void loadGlyph() { readTTF(); buildOutlines(); TMPGETBEZIER();  };
};