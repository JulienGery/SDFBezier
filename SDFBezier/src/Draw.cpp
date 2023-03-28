#include "Draw.h"

#include <execution>
#include <iostream>
#include <algorithm>

void drawSq(const glm::vec2& location, const size_t& width, const size_t& height, uint32_t* array, const size_t& length, const size_t& color)
{
	size_t x = location.x * width;
	size_t y = location.y * height;
	const float l2 = (float)(length - 1) / 2;

	for (size_t i = 0; i < length; i++)
	{
		const size_t cy = y + i - l2;
		if (cy < height)
		for (size_t j = 0; j < length; j++)
		{
			const size_t cx = x + j - l2;
			if (cx < width)
			{
				const size_t index = cy * width + cx;
				array[index] = color;
			}	
		}
		
	}

}

void drawCurve(const Bezier& curve, const uint32_t& width, const uint32_t& height, uint32_t* array, const uint32_t& color)
{
	const size_t count = curve.m_Count;
	for (size_t i = 0; i < count + 1; i++)
	{
		glm::vec2 point = curve((float) i / count);

		const uint32_t x = point.x * width;
		const uint32_t y = point.y * height;

		if (x > width || y > height)
			continue;

		size_t index = y * width + x;
		if (index < width * height)
			array[index] = color;
	}
}

void drawDerivate(const Bezier& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	const size_t count = curve.m_Count;
	for (size_t i = 0; i < count + 1; i++)
	{
		glm::vec2 point = curve.derivate((float)i / count);
		
		const uint32_t x = point.x * width;
		const uint32_t y = point.y * height;

		if (x > width || y > height)
			continue;

		size_t index = y * width + x;
		if (index < width * height)
			array[index] = 0xffff00ff;
	}
}

void RenderCurve(const Bezier& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	const glm::vec2 bottomLeft = curve.getBottomLeft();
	const glm::vec2 topRight = curve.getTopRight();
	const size_t curveHeght = (topRight.y - bottomLeft.y) * height + 1;
	std::vector<size_t> vecHeight(curveHeght);
	std::iota(vecHeight.begin(), vecHeight.end(), bottomLeft.y * height);

	std::for_each(std::execution::par, vecHeight.begin(), vecHeight.end(),
		[&curve, &width, &height, &array](const size_t& yi)
		{
			const float y = (float)yi / (float)height;
			size_t xi = 0;
			double start = 0;

			for (float x = 0; x < 1.f; x += 1.0f / (float)width)
			{
				glm::vec2 point{ x, y };
				size_t index = yi * width + xi;
				xi++;

				const float distance = curve.findClosestPoint(point, start).distance;
				if (distance < 1e-05f)
					array[index] = 0xff0000ff;
			}
		});
}


void renderGlyph(const Glyph& glyph, const size_t& width, const size_t& height, uint32_t* array)
{
	//const glm::vec2 bottomLeft = glyph.getBottomLeft();
	//const glm::vec2 topRight = glyph.getTopRight();
	//size_t glyphHeight = (topRight.y - bottomLeft.y) * height;

	std::vector<size_t> vecHeight(height);
	std::iota(vecHeight.begin(), vecHeight.end(), 0);

	std::for_each(std::execution::par, vecHeight.begin(), vecHeight.end(),
		[&glyph, &width, &height, &array](const size_t& yi)
		{
			const float y = (float)yi / (float)height;
			double start = 0;

			for (size_t x = 0; x < width; x++)
			{
				const glm::vec2 point{ x / (float)width, y};
				const size_t xi = x;
				const size_t index = yi * width + xi;
				
				//if (!(index < width * height)) continue;
				if (glyph.inside(point, start))
					array[index] = 0xff00ff00;
			}
		});
}