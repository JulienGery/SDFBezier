#pragma once

#include <vector>
#include <glm/vec2.hpp>

class Bezier
{
public:
	std::vector<glm::vec2> m_Points;

	Bezier(const std::vector<glm::vec2>& points);
	Bezier(const glm::vec2 point);

	std::vector<glm::vec2> getVectors() const;

	void addPoint(const glm::vec2 point);

	glm::vec2 endDerivate() const;

	glm::vec2 startDerivate() const;

	glm::vec2 getLastPoint() const;

	glm::vec2 getFirstPoint() const { return m_Points[0]; }

	size_t size() const { return m_Points.size(); }

};