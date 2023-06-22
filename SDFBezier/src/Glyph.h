#pragma once

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

struct Bezier
{
	std::vector<glm::vec2> m_Points;
	
	Bezier(const std::vector<glm::vec2>& points)
	{
		if (points.size() > 4)
			throw std::runtime_error("failed to create Bezier");
		m_Points = points;
	}

	std::vector<glm::vec2> getVectors() const
	{
		const glm::vec2 P_0 = m_Points[0];
		const glm::vec2 P_1 = m_Points[1];
		const glm::vec2 P_2 = m_Points.size() > 2 ? m_Points[2] : glm::vec2{ 0, 0 };
		const glm::vec2 P_3 = m_Points.size() > 3 ? m_Points[3] : glm::vec2{ 0, 0 };

		return {
			P_0,
			P_1 - P_0,
			P_2 - 2.f * P_1 + P_0,
			P_3 - 3.f * P_2 + 3.f * P_1 - P_0
		};
	}

	void addPoint(const glm::vec2 point)
	{
		if (m_Points.size() < 4)
			m_Points.push_back(point);
		else
			throw std::runtime_error("failed to add point to bezier");
	}

	glm::vec2 getDerivateEnd() const
	{
		const size_t end = m_Points.size() - 1;

		const glm::vec2 endVec = m_Points[end];
		const glm::vec2 preEnd = m_Points[end - 1];

		return 2.0f * (endVec - preEnd);
	}

	glm::vec2 getStartDerivate() const
	{
		const glm::vec2 start = m_Points[0];
		const glm::vec2 sec = m_Points[1];

		return 2.0f*(sec - start);
	}

	glm::vec2 getLastPoint() const
	{
		const size_t end = m_Points.size() - 1;

		return m_Points[end];
	}

	glm::vec2 getFirstPoint() const { return m_Points[0]; }

	size_t size() const { return m_Points.size(); }
	
};

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
	OutLines m_Outlines;
	std::vector<Bezier> m_Curves;
	std::vector<glm::vec4> m_Bisectors;
	std::string m_Path;
	std::vector<float> m_Angles;

	Glyph(const std::string& path, const char& character);

	glm::vec2 getBottomLeft() const { return m_BottomLeft; }
	glm::vec2 getTopRight() const { return m_TopRight; }
	glm::vec2 getBarycentre() const { return m_Barycentre; }


private:
	glm::vec2 m_BottomLeft, m_TopRight, m_Barycentre;
	std::vector<uint16_t> m_Split;

	void readTTF();
	void BuildCurves();
	void generateBisectors();
	void loadGlyph() { readTTF();  BuildCurves(); generateBisectors(); };
};