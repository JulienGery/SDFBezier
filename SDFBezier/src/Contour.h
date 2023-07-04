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
	glm::vec4 m_bbox{0.};

	Contour() { m_Curves.clear(); m_Bisector.clear(); }
	~Contour() {}

	void push_back(const Bezier& bezier);
	void reverse();
	void scale(const float c);
	void buildBisector();
	void computeBbox();

private:
};