#pragma once

#include <vector>
#include <complex>
#include <string>

#include "glm/geometric.hpp"

class Poly {
public:
	std::vector<double> m_Coefs;
	std::string str();

	Poly (const std::vector<double>& coefs);
	template <class T>
	T operator()(const T& t);
	
};

// dirty way to find roots of quintic.
// DOTO Rework Poly
class Quintic : public Poly
{
public:
	Quintic(const std::vector<double>& coefs);
	std::vector<std::complex<double>> roots(double& start, const std::vector<float>& startingPoints);

	template <class T>
	T derivate(const T& t);
	template <class T>
	T derivateSecond(const T& t);
	template <class T>
	T derivateThird(const T& t);
	template <class T>
	T derivateFourth(const T& t);

private:
	double findStartingPoint(const std::vector<float>& startingPoints);
};

class Quartic : public Poly
{
public:
	Quartic(const std::vector<double>& coefs);
	std::vector<std::complex<double>> roots();
};

class Cubic : public Poly
{
public:
	Cubic(const std::vector<double>& coefs);
	std::vector<std::complex<double>> roots();
};

class Quadratic : public Poly
{
public:
	Quadratic(const std::vector<double>& coef);
	std::vector<double> roots();
};