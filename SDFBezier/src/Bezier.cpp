#include "Bezier.h"
#include "glm/geometric.hpp"

#include <execution>
#include <algorithm>

inline float distanceSq(const glm::vec2& a, const glm::vec2& b)
{
	glm::vec2 tmp = a - b;
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

	// f(t) = 
	// P_0 +
	// 3t(P_1 - P_0) +
	// 3t^2(P_2 - 2P_1 + P_0) +
	// t^3(P_3 - 3P_2 + 3P_1 - P_0)

	// dt = 
	// 3(P_1 - P_0) + 
	// 6t * (P_2 - 2P_1 + P_0)
	// 3t^2(P_3 - 3P_2 + 3P_1 - P_0)

	// 1/3 dt =
	// P_1 - P_0 +
	// 2t * (P_2 - 2P_1 + P_0) + 
	// t^2(P_3 - 3P_2 + 3P_1 - P_0)

	// p = P - P_0
	// p_1 = P_1 - P_0
	// p_2 = P_2 - 2P_1 + P_0
	// p_3 = P_3 - 3P_2 + 3P_1 - P_0

	// f(t) - P = 
	// -p + 3tp_1 + 3t^2p_2 + t^3p_3 

	// 1/3 dt = p_1 + 2tp_2 + t^2p_3

	// (f - P)dt = 0
	// (f - P)1/3 dt = 0
	// (-p + 3tp_1 + 3t^2p_2 + t^3p_3)(p_1 + 2tp_2 + t^2p_3) = 0
	
	// (-p)(p_1) + (-p)(2tp_2) + (-p)(t^(2) p_3) + (3tp_1)(p_1) + (3tp_1)(2tp_2) + (3tp_1)(t^(2) p_3) + (3t^(2) p_2)(p_1) + (3t^(2) p_2)(2tp_2) + (3t^(2) p_2)(t^(2) p_3) + (t^(3) p_3)(p_1) + (t^(3) p_3)(2tp_2) + (t^(3) p_3)(t^(2) p_3)
	
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

	size_t size = 0;
	std::vector<double> realRoots(roots.size());
	for (const auto& i : roots)
		if (std::abs(i.imag()) < 1e-08)
		{
			realRoots[size] = glm::clamp(i.real(), 0.0, 1.0);
			size++;
		}

	realRoots.resize(size); // faster than push_back

	glm::vec2 closestPoint = this->operator()(realRoots[0]);
	float distance = distanceSq(closestPoint, point);
	for (size_t i = 1; i < realRoots.size(); i++)
	{
		glm::vec2 testPoint = this->operator()(realRoots[i]);
		float testDistance = distanceSq(testPoint, point);
		if (testDistance < distance)
		{
			closestPoint = testPoint;
			distance = testDistance;
		}
	}

	return { distance, closestPoint};
}

void Bezier3::renderCurve(const size_t& width, const size_t& height, uint32_t* array)
{
	std::vector<size_t> vecHeight(height);
	std::iota(vecHeight.begin(), vecHeight.end(), 0);

	std::for_each(std::execution::par, vecHeight.begin(), vecHeight.end(),
		[this, &width, &height, &array](const size_t& yi)
		{
			const float y = (float)yi / (float)height;
			size_t xi = 0;
			double start = 0;

			for (float x = 0; x < 1.f; x += 1.0f / (float)width)
			{
				glm::vec2 point{ x, y };
				size_t index = yi * width + xi;

				float distance = this->findClosestPoint(point, start).distance;
				if (distance < 1e-05f)
					array[index] = 0xff0000ff;
				xi++;
			}
		});
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

Bezier1::~Bezier1() {}