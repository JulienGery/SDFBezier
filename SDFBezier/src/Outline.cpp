#include "Outline.h"
#include "glm/mat2x2.hpp"
#include "glm/trigonometric.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <complex>

#define _USE_MATH_DEFINES
#include <math.h> // only for pi

Outline::Outline(const std::vector<glm::vec2>& points, const std::vector<uint16_t>& flags)
{
	buildCurves(points, flags);

	//edge pruning somewhere

	buildBisector();
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

void Outline::buildCurves(const std::vector<glm::vec2>& points, const std::vector<uint16_t>& flags)
{
	Bezier Curve{ points[0] };
	glm::vec2 previusPoint = points[0];
	for (size_t i = 1; i < points.size(); i++)
	{
		//Build curves

		const glm::vec2 point = points[i];
		const uint16_t flag = flags[i];

		if (flags[i] & FT_CURVE_TAG_ON)
		{
			Curve.addPoint(point);
			m_Curves.push_back(Curve);

			Curve = Bezier{ point };
		}
		else if (!(flag & FT_CURVE_TAG_CUBIC) && Curve.size() == 2)
		{
			const glm::vec2 middle = (previusPoint + point) / 2.0f;

			Curve.addPoint(middle);
			m_Curves.push_back(Curve);

			Curve = Bezier{ middle };
			Curve.addPoint(point);
		}
		else
			Curve.addPoint(point);

		previusPoint = point;
	}

	const glm::vec2 firtPoint = points[0];
	Curve.addPoint(firtPoint);
	m_Curves.push_back(Curve);
}

void Outline::buildBisector()
{
	m_Bisector.resize(m_Curves.size());

	glm::vec2 startBisector = bisector(
		-m_Curves[m_Curves.size() - 1].endDerivate(), m_Curves[0].startDerivate()
	);

	for (size_t i = 0; i < m_Curves.size(); i++)
	{
		const glm::vec2 endBisector = bisector(
			-m_Curves[i].endDerivate(), m_Curves[(i + 1) % m_Curves.size()].startDerivate()
		);

		m_Bisector[i] = {startBisector, endBisector};
		startBisector = endBisector;
	}

}
