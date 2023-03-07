#include "Bezier.h"
#include "Poly.h"
#include "glm/geometric.hpp"

#include <iostream>

inline float distanceSq(const glm::vec2& a, const glm::vec2& b)
{
	const glm::vec2 tmp = a - b;
	return glm::dot(tmp, tmp);
}

Custom Bezier::getClosestPoint(const std::vector<std::complex<double>>& roots, const glm::vec2& point)
{
	size_t size = 0;
	std::vector<double> realRoots(roots.size());
	for (const auto& i : roots)
		if (std::abs(i.imag()) < 1e-08)
		{
			realRoots[size] = glm::clamp(i.real(), 0.0, 1.0);
			size++;
		}

	glm::vec2 closestPoint = operator()(realRoots[0]);
	float distance = distanceSq(closestPoint, point);
	for (size_t i = 1; i < size; i++)
	{
		glm::vec2 testPoint = operator()(realRoots[i]);
		float testDistance = distanceSq(testPoint, point);
		if (testDistance < distance)
		{
			closestPoint = testPoint;
			distance = testDistance;
		}
	}

	return { distance, closestPoint };
}

glm::vec2 Bezier::Bezier3(const float& t)
{
	return m_P_0 +
		3.f * t * (m_P_1 - m_P_0) +
		3.f * t * t * (m_P_2 - 2.f * m_P_1 + m_P_0) +
		t * t * t * (m_P_3 - 3.f * m_P_2 + 3.f * m_P_1 - m_P_0);
	// (p_3p_3)t^5 + 
	// 5p_2p_3t^4 + 
	// (4p_1p_3 + 6p_2 p_2)t^3 + 
	// (-pp_3 + 9p_1p_2)t^2 + 
	// (-2pp_2 + 3p_1p_1)t 
	// -pp_1
}

glm::vec2 Bezier::Bezier3Derivate(const float& t)
{
	return 3.f * (m_P_1 - m_P_0) 
		+ 6.f * t * (m_P_2 - 2.f * m_P_1 + m_P_0) 
		+ 3.f * t * t * (m_P_3 - 3.f * m_P_2 + 3.f * m_P_1 - m_P_0);
}

Custom Bezier::Bezier3FindClosestPoint(const glm::vec2& point, double& start)
{
	const glm::vec2 p = point - m_P_0;
	const glm::vec2 p1 = m_P_1 - m_P_0;
	const glm::vec2 p2 = m_P_2 - 2.f * m_P_1 + m_P_0;
	const glm::vec2 p3 = m_P_3 - 3.f * m_P_2 + 3.f * m_P_1 - m_P_0;

	const float a = glm::dot(p3, p3);
	const float b = 5.f * glm::dot(p2, p3);
	const float c = 4.f * glm::dot(p1, p3) + 6.f * glm::dot(p2, p2);

	// depend on P
	const float d = 9.f * glm::dot(p1, p2) - glm::dot(p3, p); // t^2
	const float e = 3.f * glm::dot(p1, p1) - 2.f * glm::dot(p2, p); // t
	const float f = -glm::dot(p1, p);

	std::vector<std::complex<double>> roots = Quintic{ {f, e, d, c, b, a} }.roots(start);

	return getClosestPoint(roots, point);
}

void Bezier::Bezier3Extremum()
{
	const glm::vec2 p1 = m_P_1 - m_P_0;
	const glm::vec2 p2 = m_P_2 - 2.f * m_P_1 + m_P_0;
	const glm::vec2 p3 = m_P_3 - 3.f * m_P_2 + 3.f * m_P_1 - m_P_0;

	for (const auto& i : Quadratic({ p1.x, 2.0 * p2.x, p3.x }).roots())
	{
		const double tmp = glm::clamp(i, 0., 1.);
		if (tmp != 0.0 && tmp != 1.0)
			m_Extremum.push_back(i);
	}

	for (const auto& i : Quadratic({ p1.y, 2.0 * p2.y, p3.y }).roots())
	{
		const double tmp = glm::clamp(i, 0., 1.);
		if (tmp != 0 && tmp != 1)
			m_Extremum.push_back(i);
	}
		

	for (size_t i = 0; i < m_Extremum.size() - 1; i++)
	{
		size_t min = i;
		for (size_t j = i + 1; j < m_Extremum.size(); j++)
			if (m_Extremum[min] > m_Extremum[j])
				min = j;
		std::swap(m_Extremum[i], m_Extremum[min]);
	}
}

glm::vec2 Bezier::Bezier2(const float& t)
{
	return m_P_0 + 
		2.f * t * (m_P_1 - m_P_0) + 
		t * t * (m_P_2 - 2.0f * m_P_1 + m_P_0);
}

glm::vec2 Bezier::Bezier2Derivate(const float& t)
{
	return 2.f * (m_P_1 - m_P_0) +
		2.0f * t * (m_P_2 - 2.0f * m_P_1 + m_P_0);
	// p2p2
	// 3p1p2
	// 2p1p1 - pp2
	// -pp1

}

Custom Bezier::Bezier2FindClosestPoint(const glm::vec2& point)
{
	const glm::vec2 p = point - m_P_0;
	const glm::vec2 p1 = m_P_1 - m_P_0;
	const glm::vec2 p2 = m_P_2 - 2.0f * m_P_1 + m_P_0;

	const double a = glm::dot(p2, p2);
	const double b = 3.0 * glm::dot(p1, p2);
	const double c = 2.0 * glm::dot(p1, p1) - glm::dot(p2, p);
	const double d = -glm::dot(p1, p);

	const std::vector<std::complex<double>> roots = Cubic{ {d, c, b, a} }.roots();

	return getClosestPoint(roots, point);
}

void Bezier::Bezier2Extremum()
{
	//p1 +
		//tp2;
	const glm::vec2 p1 = m_P_1 - m_P_0;
	const glm::vec2 p2 = m_P_2 - 2.0f * m_P_1 + m_P_0;

	float r1 = -(p1.x / p2.x);
	float r2 = -(p1.y / p2.y);
	if (r1 > r2)
		std::swap(r1, r2);

	m_Extremum = {0, r1, r2, 1};
}

glm::vec2 Bezier::Bezier1(const float& t)
{
	return m_P_0 + t * (m_P_1 - m_P_0);
}

glm::vec2 Bezier::Bezier1Derivate(const float& t)
{
	return m_P_1 - m_P_0;

	// (tp1 - p) . p1 = 0

	// p1p1
	// -pp1
}

Custom Bezier::Bezier1FindClosestPoint(const glm::vec2& point)
{
	const glm::vec2 p = point - m_P_0;
	const glm::vec2 p1 = m_P_1 - m_P_0;

	const double a = glm::dot(p1, p1);
	const double b = -glm::dot(p1, p);

	const double t = glm::clamp(- b / a, 0.0, 1.0);
	const glm::vec2 closestPoint = operator()(t);

	return { distanceSq(closestPoint, point), closestPoint };
}

Bezier::Bezier(const std::vector<glm::vec2>& points, const size_t& count)
{
	m_Count = count;
	m_PointCount = points.size();

	if (m_PointCount)
		m_P_0 = points[0];
	if (m_PointCount > 1)
		m_P_1 = points[1];
	if (m_PointCount > 2)
		m_P_2 = points[2];
	if (m_PointCount == 4)
		m_P_3 = points[3];
}

Bezier::Bezier(const glm::vec2& point, const size_t& count)
{
	m_P_0 = point;
	m_Count = count;
	m_PointCount = 1;
}

Bezier::Bezier(const size_t& count)
{
	m_PointCount = 0;
	m_Count = count;
}

Bezier::~Bezier()
{
}

glm::vec2 Bezier::operator()(const float& t)
{
	if (m_PointCount == 4)
		return Bezier3(t);
	if (m_PointCount == 3)
		return Bezier2(t);
	return Bezier1(t);
}

glm::vec2 Bezier::derivate(const float& t)
{
	if (m_PointCount == 4)
		return Bezier3Derivate(t);
	if (m_PointCount == 3)
		return Bezier2Derivate(t);
	return Bezier1Derivate(t);
}

Custom Bezier::findClosestPoint(const glm::vec2& point, double& start)
{
	if (m_PointCount == 4)
		return Bezier3FindClosestPoint(point, start);
	if (m_PointCount == 3)
		return Bezier2FindClosestPoint(point);
	return Bezier1FindClosestPoint(point);
}

void Bezier::addPoint(const glm::vec2& point)
{
	if (m_PointCount == 4)
		throw std::runtime_error("failed to add point to bezier");

	m_PointCount++;
	if (m_PointCount == 1)
		m_P_0 = point;
	else if (m_PointCount == 2)
		m_P_1 = point;
	else if (m_PointCount == 3)
		m_P_2 = point;
	else if (m_PointCount == 4)
		m_P_3 = point;
}

void Bezier::updateExtremum()
{
	m_Extremum.clear();
	m_Extremum = { 0, 1 };
	if (m_PointCount == 4)
		Bezier3Extremum();
	else if (m_PointCount == 3)
		Bezier2Extremum();

	m_ExtremumPoints.resize(m_Extremum.size());
	for (size_t i = 0; i < m_ExtremumPoints.size(); i++)
		m_ExtremumPoints[i] = operator()(m_Extremum[i]);

	computeBoundingBox();
}


void Bezier::computeBoundingBox()
{

	m_TopRight = m_ExtremumPoints[0];
	m_BottomLeft = m_ExtremumPoints[0];
	
	for (const auto& i : m_ExtremumPoints)
	{
		if (i.x > m_TopRight.x)
			m_TopRight.x = i.x;
		if (i.y > m_TopRight.y)
			m_TopRight.y = i.y;
		if (i.x < m_BottomLeft.x)
			m_BottomLeft.x = i.x;
		if (i.y < m_BottomLeft.y)
			m_BottomLeft.y = i.y;
	}
}