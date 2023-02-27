#pragma once
#include "glm/vec2.hpp"
#include "Bezier.h"

void drawSq(const glm::vec2& location, const size_t& width, const size_t& height, uint32_t* array, const size_t& length, const size_t& color);

void drawCurve(Bezier3& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);
void drawCurve(Bezier2& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);
void drawCurve(Bezier1& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);

void drawDerivate(Bezier3& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);

void RenderCurve(Bezier3& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);
void RenderCurve(Bezier2& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);
void RenderCurve(Bezier1& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);