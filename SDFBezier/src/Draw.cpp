#include "Draw.h"

#include <execution>
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

void drawCurve(Bezier3& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	for (size_t i = 0; i < curve.m_Count + 1; i++)
	{
		glm::vec2 point = curve((float) i / curve.m_Count);

		const uint32_t x = point.x * width;
		const uint32_t y = point.y * height;

		if (x > width || y > height)
			continue;

		size_t index = y * width + x;
		if (index < width * height)
			array[index] = 0xffff00ff;
	}
}
void drawCurve(Bezier2& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	for (size_t i = 0; i < curve.m_Count + 1; i++)
	{
		glm::vec2 point = curve((float)i / curve.m_Count);

		const uint32_t x = point.x * width;
		const uint32_t y = point.y * height;

		if (x > width || y > height)
			continue;

		size_t index = y * width + x;
		if (index < width * height)
			array[index] = 0xffff00ff;
	}
}
void drawCurve(Bezier1& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	for (size_t i = 0; i < curve.m_Count + 1; i++)
	{
		glm::vec2 point = curve((float)i / curve.m_Count);

		const uint32_t x = point.x * width;
		const uint32_t y = point.y * height;

		if (x > width || y > height)
			continue;

		size_t index = y * width + x;
		if (index < width * height)
			array[index] = 0xffff00ff;
	}
}

void drawDerivate(Bezier3& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	for (size_t i = 0; i < curve.m_Count + 1; i++)
	{
		glm::vec2 point = curve.derivate((float)i / curve.m_Count);
		
		
		const uint32_t x = point.x * width;
		const uint32_t y = point.y * height;

		if (x > width || y > height)
			continue;

		size_t index = y * width + x;
		if (index < width * height)
			array[index] = 0xffff00ff;
	}
}

void RenderCurve(Bezier3& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	std::vector<size_t> vecHeight(height);
	std::iota(vecHeight.begin(), vecHeight.end(), 0);

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

				float distance = curve.findClosestPoint(point, start).distance;
				if (distance < 1e-05f)
					array[index] = 0xff0000ff;
				xi++;
			}
		});
}

void RenderCurve(Bezier2& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	std::vector<size_t> vecHeight(height);
	std::iota(vecHeight.begin(), vecHeight.end(), 0);

	std::for_each(std::execution::par, vecHeight.begin(), vecHeight.end(),
		[&curve, &width, &height, &array](const size_t& yi)
		{
			const float y = (float)yi / (float)height;
			size_t xi = 0;

			for (float x = 0; x < 1.f; x += 1.0f / (float)width)
			{
				glm::vec2 point{ x, y };
				size_t index = yi * width + xi;

				float distance = curve.findClosestPoint(point).distance;
				if (distance < 1e-05f)
					array[index] = 0xff0000ff;
				xi++;
			}
		});
}

void RenderCurve(Bezier1& curve, const uint32_t& width, const uint32_t& height, uint32_t* array)
{
	std::vector<size_t> vecHeight(height);
	std::iota(vecHeight.begin(), vecHeight.end(), 0);

	std::for_each(std::execution::par, vecHeight.begin(), vecHeight.end(),
		[&curve, &width, &height, &array](const size_t& yi)
		{
			const float y = (float)yi / (float)height;
			size_t xi = 0;

			for (float x = 0; x < 1.f; x += 1.0f / (float)width)
			{
				glm::vec2 point{ x, y };
				size_t index = yi * width + xi;

				float distance = curve.findClosestPoint(point).distance;
				if (distance < 1e-05f)
					array[index] = 0xff0000ff;
				xi++;
			}
		});
}