#pragma once

#include <variant>
#include <vector>

#include "glm/vec2.hpp"

struct Custom // tmp struc will be removed
{
	float distance;
	glm::vec2 point;
};

//need to find a way to have only one class for all of bezier

class Bezier3
{
public:
	glm::vec2 m_P_0, m_P_1, m_P_2, m_P_3;

	size_t m_Count = 100;

	Bezier3(const glm::vec2& p_0, const glm::vec2& p_1, const glm::vec2& p_2, const glm::vec2& p_3, const size_t& m_Count);
	~Bezier3();

	glm::vec2 operator() (const float& t);
	glm::vec2 derivate(const float& t);
	Custom findClosestPoint(const glm::vec2& point, double& start);
};

class Bezier2
{
public:
	glm::vec2 m_P_0, m_P_1, m_P_2;
	size_t m_Count = 100;

	Bezier2(const glm::vec2& p_0, const glm::vec2& p_1, const glm::vec2& p_2, const size_t& m_Count);
	~Bezier2();

	glm::vec2 operator() (const float& t);
	glm::vec2 derivate(const float& t);
	Custom findClosestPoint(const glm::vec2& point);

};

class Bezier1
{
public:
	glm::vec2 m_P_0, m_P_1;
	size_t m_Count = 100;

	Bezier1(const glm::vec2& p_0, const glm::vec2& p_1, const size_t& m_Count);
	~Bezier1();

	glm::vec2 operator() (const float& t);
	glm::vec2 derivate(const float& t);
	Custom findClosestPoint(const glm::vec2& point);
};

class Bezier
{
public:
	Bezier(const glm::vec2& p_0, const glm::vec2& p_1, const size_t& m_Count) : m_Impl(Bezier1(p_0, p_1, m_Count)) {}
	Bezier(const glm::vec2& p_0, const glm::vec2& p_1, const glm::vec2& p_2, const size_t& m_Count) : m_Impl(Bezier2(p_0, p_1, p_2, m_Count)) {}
	Bezier(const glm::vec2& p_0, const glm::vec2& p_1, const glm::vec2& p_2, const glm::vec2& p_3, const size_t& m_Count) : m_Impl(Bezier3(p_0, p_1, p_2, p_3, m_Count)) {}

	glm::vec2 operator() (const float& t);
	glm::vec2 derivate(const float& t);
	Custom findClosestPoint(const glm::vec2& point, double& start);

	size_t getCount();

	std::vector<glm::vec2*> tmp();

private:
	std::variant<Bezier1, Bezier2, Bezier3> m_Impl;
};