#include "Poly.h"

#include <iostream>

Poly::Poly(const std::vector<double>& coefs) : m_Coefs{ coefs }
{}

template <class T>
T Poly::operator()(const T& t)
{
	T result = m_Coefs[0];
	T x = 1;
	for (size_t i = 1; i < m_Coefs.size(); i++)
	{
		x *= t;
		result += m_Coefs[i] * x;
	}
	return result;
}

template <class T>
T Quintic::derivate(const T& t)
{
	T result = m_Coefs[1];
	T x = 1;
	for (size_t i = 2; i < m_Coefs.size(); i++)
	{
		x *= t;
		result += m_Coefs[i] * x * i;
	}
	return result;
}

template <class T>
T Quintic::derivateSecond(const T& t)
{
	T result = m_Coefs[2] * 2;
	T x = 1;
	for (size_t i = 3; i < m_Coefs.size(); i++)
	{
		x *= t;
		result += m_Coefs[i] * i * (i - 1) * x;
	}
	return result;
}

template <class T>
T Quintic::derivateThird(const T& t)
{
	T result = m_Coefs[3] * 6;
	T x = 1;
	for (size_t i = 4; i < m_Coefs.size(); i++)
	{
		x *= t;
		result += m_Coefs[i] * i * (i - 1) * (i - 2) * x;
	}
	
	return result;
}

template <class T>
T Quintic::derivateFourth(const T& t)
{
	return m_Coefs[4] * 24 + m_Coefs[5] * 120 * t;
}

std::string Poly::str()
{
	std::string result = "";
	
	for (size_t i = 0; i < m_Coefs.size(); i++)
		result += std::to_string(m_Coefs[i]) + "x^" + std::to_string(i) + '+';
	return result;
}

Quintic::Quintic(const std::vector<double>& coef) : Poly{ coef }
{}

std::vector<std::complex<double>> Quintic::roots(double& start)
{

	double a = m_Coefs[5];
	double b = m_Coefs[4];
	double c = m_Coefs[3];
	double d = m_Coefs[2];
	double e = m_Coefs[1];
	double g = m_Coefs[0];

	b /= a;
	c /= a;
	d /= a;
	e /= a;
	g /= a;
	a = 1.0;

	double f;
	size_t count = 0;

	while (true)
	{
		if (count == 10) 
			start = startingPoint(e, d, c, b, a);

		f = this->operator()(start);
		if (glm::abs(f) < 1e-08) break;

#define halley 1
#if halley
		//halley method

		const double fp = this->derivate(start);
		const double fpp = this->derivateSecond(start);

		start -= 2.f * f * fp / (2.f * fp * fp - f * fpp);
#else
		//householder - 4
		// no meaningful difference.

		const double fp = this->derivate(start);
		const double fpp = this->derivateSecond(start);
		const double fppp = this->derivateThird(start);
		const double fpppp = this->derivateFourth(start);

		const double f2 = f * f;
		const double f3 = f2 * f;

		const double fp2 = fp * fp;
		const double fp3 = fp2 * fp;

		start += 4.0 * (6.0 * f2 * fp * fpp - 6.0 * f * fp3 - f3 * fppp) /
			(24.0 * fp2 * fp2 + 6.0 * f2 * fpp * fpp - 36.0 * f * fp2 * fpp + 8.0 * f2 * fp * fppp - f3 * fpppp);

#endif
		count++;
	}

	double k, l, m, n;

	if (start != 0.)
	{
		k = b + start;
		l = c + start * k;
		m = d + start * l;
		n = -g / start;
	}
	else
	{
		k = b;
		l = c;
		m = d;
		n = e;
	}

	Quartic quartic{ {n, m, l, k, a} };
	
	std::vector<std::complex<double>> result = quartic.roots();
	result.push_back(start);

	return result;
}

double Quintic::startingPoint(const double& e, const double& d, const double& c, const double& b, const double& a)
{
	Quartic quartic({ e, d * 2., c * 3., b * 4., a * 5. });
	const std::vector<std::complex<double>> roots = quartic.roots();
	std::vector<double> realsRoots(roots.size());

	size_t size = 0;
	for(const auto& root : roots)
		if (root.imag() < 1e-08)
		{
			realsRoots[size] = root.real();
			size++;
		}

	realsRoots.resize(size);

	// sort vector

	for (size_t i = 0; i < size; i++)
	{
		size_t minIndex = i;
		for (size_t j = i + 1; j < size; j++)
		{
			if (realsRoots[minIndex] > realsRoots[j])
				minIndex = j;
		}
		std::swap(realsRoots[minIndex], realsRoots[i]);
	}

	for (size_t i = 0; i < size; i++)
		if (this->operator()(realsRoots[i]) >= 0)
		{
			if (i == 0)
				return realsRoots[0] - 20.0;
			return (realsRoots[i] + realsRoots[i - 1]) / 2.0;
		}

	return realsRoots[size - 1] + 20.0;
}

Quartic::Quartic(const std::vector<double>& coef) : Poly{ coef }
{}

std::vector<std::complex<double>> Quartic::roots()
{
	double a = m_Coefs[4];
	double b = m_Coefs[3];
	double c = m_Coefs[2];
	double d = m_Coefs[1];
	double e = m_Coefs[0];

	b /= a;
	c /= a;
	d /= a;
	e /= a;
	a = 1;

	Cubic rc{ {4.0 * c * e - d * d - b * b * e, d * b - 4.0 * e, -c, 1.0} };
	std::vector<std::complex<double>> rcRoots = rc.roots();

	std::complex<double> tmp = rcRoots[0];
	for (const auto& i : rcRoots)
		if (std::abs(i.imag()) < std::abs(tmp.imag()))
			tmp = i;

	const double y = tmp.real();
	std::complex<double> R = std::sqrt(
		std::complex<double>(1.0 / 4.0 * b * b - c + y)
	);

	std::complex<double> D, E;

	if (std::abs(R) < 1e-08)
	{
		R = 0;
		const std::complex<double> putain = 3.0 / 4.0 * b * b - 2.0 * c;
		const std::complex<double> jsp = 2.0 * std::sqrt(std::complex<double>(y * y - 4.0 * e));

		D = std::sqrt(putain + jsp);
		E = std::sqrt(putain - jsp);
	}
	else
	{
		const std::complex<double> putain = 3.0 / 4.0 * b * b - R * R - 2.0 * c;
		const std::complex<double> jsp = (4.0 * b * c - 8.0 * d - b * b * b) / (4.0 * R);

		D = std::sqrt(putain + jsp);
		E = std::sqrt(putain - jsp);
	}


	const std::complex<double> min4d = - b / 4.0;
	const std::complex<double> R2 = R / 2.0;
	const std::complex<double> D2 = D / 2.0;
	const std::complex<double> E2 = E / 2.0;

	std::vector<std::complex<double>> result {
		min4d + R2 + D2,
		min4d + R2 - D2,
		min4d - R2 + E2,
		min4d - R2 - E2
	};

	// Correction halley just in case

	/*for (auto& i : result)
	{
		std::complex<double> f, fp, fpp;
		f = this->operator()(i);
		fp = this->derivate(i);
		fpp = this->derivateSecond(i);
		i -= 2.0 * f * fp / (2.0 * fp * fp - f * fpp);
	}*/
	return result;
}


Cubic::Cubic(const std::vector<double>& coef) : Poly{ coef }
{}

std::vector<std::complex<double>> Cubic::roots()
{
	std::vector<std::complex<double>> result(3);

	double a = m_Coefs[3];
	double b = m_Coefs[2];
	double c = m_Coefs[1];
	double d = m_Coefs[0];

	b /= a;
	c /= a;
	d /= a;
	a = 1;

	const double p = (3.0 * c - b * b) / 3.0;
	const double q = (9.0 * c * b - 27.0 * d - 2. * b * b * b) / 27.0;

	const double R = q / 2.0;
	const double Q = p / 3.0;

	const std::complex<double> w3 = R + std::sqrt(
		std::complex<double>(R * R + Q * Q * Q)
	);

	const double length = std::pow(std::abs(w3), 1.0/3.0);
	const double o = std::arg(w3);

	for (size_t i = 0; i < 3; i++)
	{
		const std::complex<double> w = length * std::exp(std::complex<double>(0, (o + 2.0 * i * M_PI) / 3.0));
		const std::complex<double> x = w - p / (3.0 * w);
		result[i] = x - b / 3.0;
	}
	return result;
}