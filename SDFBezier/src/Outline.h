#pragma once

#include "Bezier.h"
#include "glm/vec4.hpp"

#include <vector>

#include <iostream>
#include <stdexcept>

class Contour
{
public:
	std::vector<Bezier> m_Curves; // TMP
	std::vector<glm::vec4> m_Bisector; // TMP

	Contour(const std::vector<glm::vec2>& points, const std::vector<uint16_t>& flag);
	~Contour() {}

private:

	void buildCurves(const std::vector<glm::vec2>& points, const std::vector<uint16_t>& flags);
	void buildBisector();
};