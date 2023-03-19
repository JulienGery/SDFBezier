#pragma once
#include "glm/vec2.hpp"
#include "Bezier.h"
#include "Glyph.h"

void drawSq(const glm::vec2& location, const size_t& width, const size_t& height, uint32_t* array, const size_t& length, const size_t& color);

void drawCurve(const Bezier& curve, const uint32_t& width, const uint32_t& height, uint32_t* array, const uint32_t& color = 0xffff0000);

void drawDerivate(Bezier& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);

void RenderCurve(const Bezier& curve, const uint32_t& width, const uint32_t& height, uint32_t* array);

void renderGlyph(const Glyph& glyph, const size_t& width, const size_t& height, uint32_t* array);