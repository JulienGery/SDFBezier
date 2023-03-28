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

	glm::vec2 getBottomLeft() const { return m_BottomLeft; }
	glm::vec2 getTopRight() const { return m_TopRight; }
	glm::vec2 getBarycentre() const { return m_Barycentre; }

	IndexRootDistancePoint findClosestPoint(const glm::vec2& point, double& start) const;
	bool inside(const glm::vec2& point, double& start) const;

private:
	glm::vec2 m_BottomLeft, m_TopRight, m_Barycentre;

	OutLines readTTF();
	void loadGlyph() { BuildCurves(readTTF()); computeBaryCentre(); };
	void BuildCurves(const OutLines& jsp);
	void computeBaryCentre();
};