#pragma once

#include "glm/vec2.hpp"
#include "Poly.h"

struct Custom // tmp struc will be removed
{
	float distance;
	glm::vec2 point;
};

inline float distanceSq(const glm::vec2& a, const glm::vec2& b);

class Bezier3 
{
public:
	glm::vec2 m_P_0, m_P_1, m_P_2, m_P_3;

	size_t m_Count = 100;

	Bezier3(const glm::vec2& p_0, const glm::vec2& p_1, const glm::vec2& p_2, const glm::vec2& p_3, const size_t& m_Count);
	~Bezier3();

	glm::vec2 operator() (const float& t);
	Custom findClosestPoint(const glm::vec2& point, double& start);
	glm::vec2 derivate(const float& t);

	void renderCurve(const size_t& width, const size_t& height, uint32_t* array);
};

class Bezier2
{
public:
	glm::vec2 m_P_0, m_P_1, m_P_2;
	size_t m_Count = 100;

	Bezier2(const glm::vec2& p_0, const glm::vec2& p_1, const glm::vec2& p_2, const size_t& m_Count);
	~Bezier2();

	glm::vec2 operator() (const float& t);
};


class Bezier1
{
public:
	glm::vec2 m_P_0, m_P_1;
	size_t m_Count = 100;

	Bezier1(const glm::vec2& p_0, const glm::vec2& p_1, const size_t& m_Count);
	~Bezier1();

	glm::vec2 operator() (const float& t);
};