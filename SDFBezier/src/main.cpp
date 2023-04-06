#include "SolveQuinticGPU.h"

#include <iostream>


void coutCoeff(const Coeff coeff)
{
	for (size_t i = 0; i < 6; i++)
		std::cout << coeff._coeff[i].x << ' ' << coeff._coeff[i].y << "  ";
	std::cout << '\n';
}

void coutRoot(const Roots root)
{
	for (size_t i = 0; i < 5; i++)
		std::cout << root._roots[i].x << ' ' << root._roots[i].y << "  ";
	std::cout << '\n';
}

int main()
{
	SolveQuinticGPU solver{};

	solver.execute();

	const std::vector<Roots> result = solver.getResult();

	solver.execute();

	const std::vector<Roots> result2 = solver.getResult();

	for (size_t i = 0; i < result.size(); i++)
		if (!(result[i] == result2[i]))
		{
			std::cout << i << '\n';
			break;
		}
}