#include "Bezier.h"
#include "glm/geometric.hpp"

inline float distanceSq(const glm::vec2& a, const glm::vec2& b)
{
	const glm::vec2 tmp = a - b;
	return glm::dot(tmp, tmp);
}

Bezier3::Bezier3(const glm::vec2& p_0, const glm::vec2& p_1, const glm::vec2& p_2, const glm::vec2& p_3, const size_t& count)
{
	m_P_0 = p_0;
	m_P_1 = p_1;
	m_P_2 = p_2;
	m_P_3 = p_3;

	m_Count = count;
}

glm::vec2 Bezier3::operator() (const float& t)
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

glm::vec2 Bezier3::derivate(const float& t)
{
	return 3.f * (m_P_1 - m_P_0) 
		+ 6.f * t * (m_P_2 - 2.f * m_P_1 + m_P_0) 
		+ 3.f * t * t * (m_P_3 - 3.f * m_P_2 + 3.f * m_P_1 - m_P_0);
}

Custom Bezier3::getClosestPoint(const std::vector<std::complex<double>>& roots, const glm::vec2& point)
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

Custom Bezier3::findClosestPoint(const glm::vec2& point, double& start)
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

Bezier3::~Bezier3() {}

Bezier2::Bezier2(const glm::vec2& p_0, const glm::vec2& p_1, const glm::vec2& p_2, const size_t& count)
{
	m_P_0 = p_0;
	m_P_1 = p_1;
	m_P_2 = p_2;

	m_Count = count;
}

glm::vec2 Bezier2::operator() (const float& t)
{
	return m_P_0 + 
		2.f * t * (m_P_1 - m_P_0) + 
		t * t * (m_P_2 - 2.0f * m_P_1 + m_P_0);
}

glm::vec2 Bezier2::derivate(const float& t)
{
	return 2.f * (m_P_1 - m_P_0) +
		2.0f * t * (m_P_2 - 2.0f * m_P_1 + m_P_0);
	// p2p2
	// 3p1p2
	// 2p1p1 - pp2
	// -pp1

}

Custom Bezier2::findClosestPoint(const glm::vec2& point)
{
	const glm::vec2 p = point - m_P_0;
	const glm::vec2 p1 = m_P_1 - m_P_0;
	const glm::vec2 p2 = m_P_2 - 2.0f * m_P_1 + m_P_0;

	const double a = glm::dot(p2, p2);
	const double b = 3.0 * glm::dot(p1, p2);
	const double c = 2.0 * glm::dot(p1, p1) - glm::dot(p2, p);
	const double d = -glm::dot(p1, p);

	const std::vector<std::complex<double>> roots = Cubic{ {d, c, b, a} }.roots();

	size_t size = 0;
	std::vector<double> realRoots(roots.size());
	for (const auto& i : roots)
		if (std::abs(i.imag()) < 1e-08)
		{
			realRoots[size] = glm::clamp(i.real(), 0.0, 1.0);
			size++;
		}

	realRoots.resize(size);

	glm::vec2 closestPoint = operator()(realRoots[0]);
	float distance = distanceSq(closestPoint, point);
	for (size_t i = 1; i < realRoots.size(); i++)
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

Bezier2::~Bezier2() {}

Bezier1::Bezier1(const glm::vec2& p_0, const glm::vec2& p_1, const size_t& count)
{
	m_P_0 = p_0;
	m_P_1 = p_1;

	m_Count = count;
}

glm::vec2 Bezier1::operator() (const float& t)
{
	return m_P_0 + t * (m_P_1 - m_P_0);
}

glm::vec2 Bezier1::derivate(const float& t)
{
	return m_P_1 - m_P_0;

	// (tp1 - p) . p1 = 0

	// p1p1
	// -pp1
}

Custom Bezier1::findClosestPoint(const glm::vec2& point)
{
	const glm::vec2 p = point - m_P_0;
	const glm::vec2 p1 = m_P_1 - m_P_0;

	const double a = glm::dot(p1, p1);
	const double b = -glm::dot(p1, p);

	const double t = glm::clamp(- b / a, 0.0, 1.0);
	const glm::vec2 closestPoint = operator()(t);

	return { distanceSq(closestPoint, point), closestPoint };
}

Bezier1::~Bezier1() {}