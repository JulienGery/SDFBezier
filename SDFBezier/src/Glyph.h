#pragma once

#include "Bezier.h"

#include <string>
#include <vector>

struct OutLines
{
	std::vector<float> points;
	std::vector<uint8_t> flags;
	std::vector<uint16_t> contour;
};

struct IndexRootDistancePoint
{
	size_t index;
	double root;
	float distance;
	glm::vec2 point;
	bool inside;
};

class Glyph
{
public:
	char m_Character;
	std::vector<Bezier> m_Curves;
	std::string m_Path;

	Glyph(const std::string& path, const char& character);
	void loadGlyph() { BuildCurves(readTTF()); };

	glm::vec2 getBottomLeft() const { return m_BottomLeft; }
	glm::vec2 getTopRight() const { return m_TopRight; }

	IndexRootDistancePoint findClosestPoint(const glm::vec2& point, double& start) const;
	IndexRootDistancePoint inside(const glm::vec2& point, double& start) const;

private:
	glm::vec2 m_BottomLeft, m_TopRight;

	OutLines readTTF();
	void BuildCurves(const OutLines& jsp);
};