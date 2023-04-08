#include "SolveQuinticGPU.h"
#include "Bezier.h"

#include <iostream>
#include <complex>


void coutCoeff(const Coeff coeff)
{
	for (size_t i = 0; i < 6; i++)
		std::cout << coeff._coeff[i].x << "x^" << i << '+';
	std::cout << '\n';
}

void coutRoot(const Roots root)
{
	for (size_t i = 0; i < 5; i++)
		std::cout << root._roots[i].x << ' ' << root._roots[i].y << "  ";
	std::cout << '\n';
}

std::complex<double> poly(const std::complex<float> z, const Coeff& coeff)
{
	std::complex<double> result{ coeff._coeff[0].x, 0 };
	std::complex<double> tmp{ 1, 0 };
	for (size_t i = 1; i < 6; i++)
	{
		tmp *= z;
		result += tmp * double(coeff._coeff[i].x);
	}
	return result;
}

bool valid(const Roots& roots, const Coeff& coeff)
{
	for (size_t i = 0; i < 5; i++)
	{
		const std::complex<float> z{ roots._roots[i].x , roots._roots[i].y };
		if (std::abs(poly(z, coeff)) > 0.02)
		{
			std::cout << "failed at: " << z << "->" << std::abs(poly(z, coeff)) << '\n';
			return false;
		}
	}
	return true;
}

bool PutainNan(const Roots& roots)
{
	for (size_t i = 0; i < 5; i++)
		if (std::isnan(roots._roots[i].x))
			return true;
	return false;
}

int main()
{
	Bezier bezier{
		{0.5, 0.5},
		{0, 1},
		{1, 1},
		{20, 20}
	};

	const auto jsp = bezier.GetVectors();

	SolveQuinticGPU solver{};
	solver.P_0 = jsp[0];
	solver.p1 = jsp[1];
	solver.p2 = jsp[2];
	solver.p3 = jsp[3];
	
	const std::vector<Roots> result = solver.getResult();
	const std::vector<Coeff> coef = solver.getCoeff();

	bool good = true;

	for (size_t i = 0; i < result.size(); i++)
		if (!valid(result[i], coef[i]))
		{
			good = false;
			coutRoot(result[i]);
			coutCoeff(coef[i]);
			std::cin.get();
		}

	if (good)
		std::cout << "\033[32mIt's all good!\n\033[0m";

	for (size_t i = 0; i < result.size(); i++)
		if (PutainNan(result[i]))
		{
			std::cout << i << '\n';
			coutCoeff(coef[i]);
			coutRoot(result[i]);
			break;
		}

	//std::cout << "\n\n";
	//coutCoeff(coef[368628]);
	//coutRoot(result[368628]);
}