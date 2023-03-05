#pragma once

#include <variant>
#include <vector>
#include <complex>

#include "glm/vec2.hpp"

struct Custom // tmp struc will be removed
{
	float distance;
	glm::vec2 point;
};

class Bezier
{
public:
	size_t m_Count;
	glm::vec2 m_P_0 = { 0., 0. }, 
		m_P_1 = { 0., 0. },
		m_P_2 = { 0., 0. }, 
		m_P_3 = { 0., 0. };

	Bezier(const std::vector<glm::vec2>& points, const size_t& count);
	~Bezier();

	glm::vec2 operator() (const float& t);
	glm::vec2 derivate(const float& t);
	Custom findClosestPoint(const glm::vec2& point, double& start);

	size_t getPointCount() const { return m_PointCount; }

private:
	size_t m_PointCount;

	//Bezier3
	glm::vec2 Bezier3(const float& t);
	glm::vec2 Bezier3Derivate(const float& t);
	Custom Bezier3FindClosestPoint(const glm::vec2& point, double& start);

	//Bezier2
	glm::vec2 Bezier2(const float& t);
	glm::vec2 Bezier2Derivate(const float& t);
	Custom Bezier2FindClosestPoint(const glm::vec2& point);

	//Bezier1
	glm::vec2 Bezier1(const float& t);
	glm::vec2 Bezier1Derivate(const float& t);
	Custom Bezier1FindClosestPoint(const glm::vec2& point);

	Custom getClosestPoint(const std::vector<std::complex<double>>& roots, const glm::vec2& point);
};