#pragma once

#include <vector>
#include <complex>
#include <string>

#define _USE_MATH_DEFINES
#include <math.h>
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
class Quintic : public Poly
{
public:
	Quintic(const std::vector<double>& coefs);
	std::vector<std::complex<double>> roots(double& start);
	double startingPoint(const double& e, const double& d, const double& c, const double& b, const double& a);

	template <class T>
	T derivate(const T& t);
	template <class T>
	T derivateSecond(const T& t);
	template <class T>
	T derivateThird(const T& t);
	template <class T>
	T derivateFourth(const T& t);
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