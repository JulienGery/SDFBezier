#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Walnut/Image.h"

#include "glm/vec2.hpp"
#include "Bezier.h"
#include "Draw.h"

#include "Glyph.h"

float inline crossProduct(const glm::vec2& d, const glm::vec2& v)
{
	return d.x * v.y - d.y * v.x;
}

class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer()
	{
		m_ImageData = new uint32_t[960 * 540];
		m_Image = std::make_shared<Walnut::Image>(960, 540, Walnut::ImageFormat::RGBA, m_ImageData);
	}

	virtual void OnAttach() override
	{
		
	}

	virtual void OnUpdate(float ts) override
	{
		const uint32_t width = m_Image->GetWidth();
		const uint32_t height = m_Image->GetHeight();

		if (!m_RenderCurve) return;

		for (size_t i = 0; i < width * height; i++)
			m_ImageData[i] = 0x00000000;

		{
			const Walnut::ScopedTimer timer("render glyph " + std::to_string(width) + '*' + std::to_string(height) + " pixels");
			renderGlyph(m_glyph, width, height, m_ImageData);
		}

		for (const auto& i : m_glyph.m_Curves)
			drawCurve(i, width, height, m_ImageData);

		//drawCurve(m_glyph.m_Curves[m_Index], width, height, m_ImageData, 0xffff00ff);

		//double start = 0;
		//const auto result = m_glyph.inside(m_Point, start);
		//const auto& curve = m_glyph.m_Curves[result.index];
		//const glm::vec2 derivate = curve.derivate(result.root);
		//const glm::vec2 vector = m_Point - result.point;
		 
		//drawCurve(curve, width, height, m_ImageData, 0xff0000ff);
		//drawSq(result.point, width, height, m_ImageData, 21, 0xffffffff);

		//std::cout << "v: " << vector.x << ' ' << vector.y << '\n';
		//std::cout << "derivate: " << derivate.x << ' ' << derivate.y << '\n';
		//std::cout << "cross: " << crossProduct(derivate, vector) << '\n';

		//const uint32_t color = result.inside ? 0xff00ff00 : 0xffff0000;
		//drawSq(m_Point, width, height, m_ImageData, 21, color);

		//drawCurve(curve, width, height, m_ImageData, 0xff0000ff);

		for (size_t i = 0; i < (height / 2 * width); i++)
		{
			const uint32_t y = height - i / width;
			std::swap(m_ImageData[i], m_ImageData[y * width - width + (i % width)]);
		}
	}

	virtual void OnUIRender() override
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Game");

		const uint32_t windowWidth = (uint32_t) ImGui::GetContentRegionAvail().x;
		const uint32_t windowHeight = (uint32_t) ImGui::GetContentRegionAvail().y;

		if (windowWidth != m_Image->GetWidth() || windowHeight != m_Image->GetHeight())
		{
			m_Image = std::make_shared<Walnut::Image>(windowWidth, windowHeight, Walnut::ImageFormat::RGBA);
			delete[] m_ImageData;
			m_ImageData = new uint32_t[windowWidth * windowHeight];
		}
		
		m_Image->SetData(m_ImageData);

		ImGui::Image(m_Image->GetDescriptorSet(), ImVec2((float)windowWidth, (float)windowHeight));

		ImGui::End();
		ImGui::Begin("parameters");

		ImGui::InputInt("Index", (int*)&m_Index, 1, 100);

		ImGui::InputFloat("px", &m_Point.x, 0.00001f, 1.f, "%.8f");
		ImGui::InputFloat("py", &m_Point.y, 0.00001f, 1.f, "%.8f");

		ImGui::InputInt("render curve", &m_RenderCurve, 0, 1);

		ImGui::End();

		ImGui::PopStyleVar();
	}

private:
	std::shared_ptr<Walnut::Image> m_Image;
	uint32_t* m_ImageData = nullptr;
	int m_RenderCurve = 0;

	size_t m_Index = 0;

	glm::vec2 m_Point{ 0.21800001, 0.02800000 };

	Glyph m_glyph{ "..\\polices\\times.ttf", 'I' };
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "vulkan";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	return app;
}