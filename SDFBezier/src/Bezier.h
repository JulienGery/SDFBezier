#pragma once

#include <vector>
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

class Bezier
{
public:
	std::vector<glm::vec2> m_Points;
	uint32_t m_Color = 0xff00ff00;
	glm::vec4 m_bbox; //TMP should be private

	Bezier(const std::vector<glm::vec2>& points);
	Bezier(const glm::vec2 point);

	std::vector<glm::vec2> getVectors() const;

	void addPoint(const glm::vec2 point);
	void reverse();
	void scale(const float c);

	void computeBbox();
	glm::vec4 bbox() const { return m_bbox; }

	glm::vec2 endDerivate() const;

	glm::vec2 startDerivate() const;

	glm::vec2 getLastPoint() const;

	glm::vec2 getFirstPoint() const { return m_Points[0]; }

	glm::vec2 operator()(const float t) const;

	size_t size() const { return m_Points.size(); }

private:
	std::vector<glm::vec2> extremum() const;
};