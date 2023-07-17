#include "Draw.h"

#include "glm/common.hpp"

void drawSq(const glm::vec2& location, const size_t& width, const size_t& height, uint32_t* array, const size_t& length, const size_t& color)
{
	size_t x = location.x;
	size_t y = location.y;
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
