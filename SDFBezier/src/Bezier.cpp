#include "Bezier.h"

#include <stdexcept>
#include <complex>
#include "glm/common.hpp"

#define DELTA 0.001 //used when P0 = P1 => p1 = O

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

void Bezier::reverse()
{
	std::reverse(m_Points.begin(), m_Points.end());
}

void Bezier::scale(const float c)
{
	for (size_t i = 0; i < m_Points.size(); i++)
		m_Points[i] *= c;
	m_bbox *= c;
}

void Bezier::computeBbox()
{
	const auto extre = extremum();

	glm::vec2 bottomLeft = extre[0];
	glm::vec2 topRight = extre[0];

	for (const auto& i : extre)
	{
		if (i.x > topRight.x)
			topRight.x = i.x;
		if (i.y > topRight.y)
			topRight.y = i.y;

		if (i.x < bottomLeft.x)
			bottomLeft.x = i.x;
		if (i.y < bottomLeft.y)
			bottomLeft.y = i.y;
	}

	m_bbox = { bottomLeft, topRight };
}

glm::vec2 Bezier::endDerivate() const
{
	const size_t end = m_Points.size() - 1;

	const glm::vec2 endVec = m_Points[end];
	const glm::vec2 preEnd = m_Points[end - 1];
	if (preEnd == endVec)
		return (operator()(1.0) - operator()(1.0 - DELTA)) / (float)DELTA;

	return 2.0f * (endVec - preEnd);
}

glm::vec2 Bezier::startDerivate() const
{
	const glm::vec2 start = m_Points[0];
	const glm::vec2 sec = m_Points[1];
	if (start == sec)
	{
		const glm::vec2 third = m_Points[2];
		return third - start;	
	}

	return 2.0f * (sec - start);
}

glm::vec2 Bezier::getLastPoint() const
{
	const size_t end = m_Points.size() - 1;

	return m_Points[end];
}

glm::vec2 Bezier::operator()(const float t) const
{
	const auto& vectors = getVectors();
	if (size() == 2)
		return vectors[0] + t * vectors[1];
	if (size() == 3)
		return vectors[0] + 2.0f * t * vectors[1] + t * t * vectors[2];
	return vectors[0] + 3.0f * t * vectors[1] +
		3.0f * t * t * vectors[2] + t * t * t * vectors[3];
}

std::vector<std::complex<float>> quadratic(const float a, const float b, const float c)
{
	const std::complex<float> delta = std::sqrt(std::complex<float>(b * b - 4.0 * a * c));
	return { (-b - delta) / (2.f * a), (-b + delta) / (2.f * a) };
}

std::vector<glm::vec2> Bezier::extremum() const
{
	if (m_Points.size() == 2)
		return m_Points;
	
	const std::vector<glm::vec2> vec = getVectors();
	const glm::vec2 p1 = vec[1];
	const glm::vec2 p2 = vec[2];
	
	if (m_Points.size() == 3)
	{
		const float r1 = glm::clamp(-(p1.x / p2.x), 0.f, 1.f);
		const float r2 = glm::clamp(-(p1.y / p2.y), 0.f, 1.f);
		
		return { operator()(0.),
				 operator()(r1),
				 operator()(r2),
				 operator()(1.) };
	}

	const glm::vec2 p3 = vec[3];
	std::vector<glm::vec2> result{operator()(0.0)};

	//un readable
	for (const auto& i : quadratic(p1.x, 2.0 * p2.x, p3.x))
		if (i.imag() == 0)
			result.push_back(operator()(glm::clamp(i.real(), 0.f, 1.f)));

	for (const auto& i : quadratic(p1.y, 2.0 * p2.y, p3.y))
		if (i.imag() == 0)
			result.push_back(operator()(glm::clamp(i.real(), 0.f, 1.f)));

	result.push_back(operator()(1.0));

	return result;
}