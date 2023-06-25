#include "Bezier.h"

#include <stdexcept>

Bezier::Bezier(const std::vector<glm::vec2>& points)
{
	if (points.size() > 4)
		throw std::runtime_error("failed to create Bezier");
	m_Points = points;
}

Bezier::Bezier(const glm::vec2 point)
{
	m_Points.resize(1);
	m_Points[0] = point;
}

std::vector<glm::vec2> Bezier::getVectors() const
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

void Bezier::addPoint(const glm::vec2 point)
{
	if (m_Points.size() < 4)
		m_Points.push_back(point);
	else
		throw std::runtime_error("failed to add point to bezier");
}

glm::vec2 Bezier::endDerivate() const
{
	const size_t end = m_Points.size() - 1;

	const glm::vec2 endVec = m_Points[end];
	const glm::vec2 preEnd = m_Points[end - 1];

	return 2.0f * (endVec - preEnd);
}

glm::vec2 Bezier::startDerivate() const
{
	const glm::vec2 start = m_Points[0];
	const glm::vec2 sec = m_Points[1];

	return 2.0f * (sec - start);
}

glm::vec2 Bezier::getLastPoint() const
{
	const size_t end = m_Points.size() - 1;

	return m_Points[end];
}
