#include "SolveQuinticGPU.h"

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

int main()
{
	SolveQuinticGPU solver{};
	
	solver.execute();

	const std::vector<Roots> result = solver.getResult();
	const std::vector<Coeff> coef = solver.getCoeff();

	for (size_t i = 0; i < result.size(); i++)
		if (!valid(result[i], coef[i]))
		{
			coutCoeff(coef[i]);
			coutRoot(result[i]);
			std::cin.get();
		}

}