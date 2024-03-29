#include "Contour.h"
#include "glm/mat2x2.hpp"
#include "glm/trigonometric.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <complex>

#define _USE_MATH_DEFINES
#include <math.h> // only for pi
#include "Contour.h"

#define RED 0x000000ff
#define GREEN 0x0000ff00
#define BLUE 0x00ff0000
#define ALPHA 0xff000000


void Contour::push_back(const Bezier& bezier)
{
	m_Curves.push_back(bezier);
}

void Contour::reverse()
{
	for (size_t i = 0; i < m_Curves.size(); i++)
		m_Curves[i].reverse();

	std::reverse(m_Curves.begin(), m_Curves.end());
}

void Contour::scale(const float c)
{
	for (size_t i = 0; i < m_Curves.size(); i++)
		m_Curves[i].scale(c);
	m_bbox *= c;
}

glm::mat2 rotationMatrix(const float o)
{
	return {
		glm::cos(o), glm::sin(o),		// cos -sin
		-glm::sin(o),  glm::cos(o)		// sin  cos
	};
}

glm::vec2 bisector(const glm::vec2 a, const glm::vec2 b)
{
	const glm::vec2 x = glm::normalize(a);

	const std::complex<double> zx{ a.x, a.y };
	const std::complex<double> zy{ b.x, b.y };

	float o = std::arg(zy / zx) / 2.0;

	if (o > 0.)
		o -= M_PI;

	const auto matrix = rotationMatrix(o);

	return matrix * x;
}

float cross(const glm::vec2 u, const glm::vec2 v)
{
	return u.x * v.y - u.y * v.x;
}

void Contour::buildBisector()
{
	m_Bisector.resize(m_Curves.size());
	m_Angles.resize(m_Curves.size());

	glm::vec2 startBisector = bisector(
		-m_Curves[m_Curves.size() - 1].endDerivate(), m_Curves[0].startDerivate()
	);

	bool startAngleConcave = cross(-m_Curves[m_Curves.size() - 1].endDerivate(), m_Curves[0].startDerivate()) < 0 ? true : false;

	for (size_t i = 0; i < m_Curves.size(); i++)
	{
		const glm::vec2 endBisector = bisector(
			-m_Curves[i].endDerivate(), m_Curves[(i + 1) % m_Curves.size()].startDerivate()
		);

		const bool EndAngleConcave = cross(-m_Curves[i].endDerivate(), m_Curves[(i + 1) % m_Curves.size()].startDerivate()) < 0 ? true : false;

		m_Bisector[i] = {startBisector, endBisector};
		m_Angles[i] = {startAngleConcave, EndAngleConcave};

		startBisector = endBisector;
		startAngleConcave = EndAngleConcave;
	}

}

void Contour::computeBbox()
{
	std::vector<glm::vec4> bboxs(m_Curves.size());
	for (size_t i = 0; i < m_Curves.size(); i++)
	{
		m_Curves[i].computeBbox();
		bboxs[i] = m_Curves[i].bbox();
	}

	glm::vec2 bottomLeft = { bboxs[0].x, bboxs[0].y };
	glm::vec2 topRight = { bboxs[0].z, bboxs[0].w };

	for (const auto i : bboxs)
	{
		if (i.x < bottomLeft.x)
			bottomLeft.x = i.x;
		if (i.y < bottomLeft.y)
			bottomLeft.y = i.y;
		
		if (i.z > topRight.x)
			topRight.x = i.z;
		if (i.w > topRight.y)
			topRight.y = i.w;
	}

	m_bbox = { bottomLeft, topRight };
}


float zCross(const glm::vec2 u, const glm::vec2 v)
{
	return u.x * v.y - u.y * v.x;
}

#define CROSS_EPSILON 0.01

void Contour::assignColors()
{
	const uint32_t colors[] = {RED, GREEN, BLUE};

	size_t ColorIndex = 0;

	for (size_t i = 0; i < m_Curves.size(); i++)
	{
		m_Curves[i].m_Color = colors[ColorIndex];
		
		const glm::vec2 endDerivate = m_Curves[i].endDerivate();
		const glm::vec2 startNextCurveDerivate = m_Curves[(i + 1) % m_Curves.size()].startDerivate();

		if (!glm::abs(zCross(endDerivate, startNextCurveDerivate)) < CROSS_EPSILON)
			ColorIndex = (ColorIndex + 1) % 3;
	}

	if (m_Curves[0].m_Color == m_Curves[m_Curves.size() - 1].m_Color)
		m_Curves[m_Curves.size() - 1].m_Color = m_Curves[1].m_Color;

}
