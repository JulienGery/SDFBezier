#pragma once

#include <vector>

#include "glm/vec2.hpp"

struct Bezier
{
	glm::vec2 P_0{0}, P_1{0}, P_2{0}, P_3{0};

	std::vector<glm::vec2> GetVectors() const
	{
		
		return {
			P_0,
			P_1 - P_0,
			P_2 - 2.f * P_1 + P_0,
			P_3 - 3.f * P_2 + 3.f * P_1 - P_0
			};
	}
};