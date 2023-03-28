#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Walnut/Image.h"

#include "glm/vec2.hpp"
#include "Bezier.h"
#include "Draw.h"
#include "Glyph.h"
#include "SolveQuinticGPU.h"

float inline crossProduct(const glm::vec2& d, const glm::vec2& v)
{
	return d.x * v.y - d.y * v.x;
}

class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer()
	{
		solver.execute();

		m_ImageData = new uint32_t[16 * 16];
		m_Image = std::make_shared<Walnut::Image>(16, 16, Walnut::ImageFormat::RGBA, m_ImageData);
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
			//const Walnut::ScopedTimer timer("render glyph " + std::to_string(width) + '*' + std::to_string(height) + " pixels");
			//renderGlyph(m_glyph, width, height, m_ImageData);
		}

		//for (size_t i = 0; i < (height / 2 * width); i++)
		{
			//const uint32_t y = height - i / width;
			//std::swap(m_ImageData[i], m_ImageData[y * width - width + (i % width)]);
		}
	}

	virtual void OnUIRender() override
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Game");

		const uint32_t windowWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
		const uint32_t windowHeight = (uint32_t)ImGui::GetContentRegionAvail().y;

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

		//const size_t pointCount = m_Curve.getPointCount();

		//ImGui::InputFloat("p0x", &m_Curve.m_P_0.x, 0.01f, 1.f, "%.2f");
		//ImGui::InputFloat("p0y", &m_Curve.m_P_0.y, 0.01f, 1.f, "%.2f");

		//ImGui::InputFloat("p1x", &m_Curve.m_P_1.x, 0.01f, 1.f, "%.2f");
		//ImGui::InputFloat("p1y", &m_Curve.m_P_1.y, 0.01f, 1.f, "%.2f");

		//if (pointCount >= 3)
		//{
		//	ImGui::InputFloat("p2x", &m_Curve.m_P_2.x, 0.01f, 1.f, "%.2f");
		//	ImGui::InputFloat("p2y", &m_Curve.m_P_2.y, 0.01f, 1.f, "%.2f");
		//}

		//if (pointCount == 4)
		//{
		//	ImGui::InputFloat("p3x", &m_Curve.m_P_3.x, 0.01f, 1.f, "%.2f");
		//	ImGui::InputFloat("p3y", &m_Curve.m_P_3.y, 0.01f, 1.f, "%.2f");
		//}

		ImGui::InputInt("Index", (int*)&m_Index, 1, 100);

		ImGui::InputFloat("px", &m_Point.x, 0.01f, 1.f, "%.2f");
		ImGui::InputFloat("py", &m_Point.y, 0.01f, 1.f, "%.2f");

		ImGui::InputInt("render curve", &m_RenderCurve, 0, 1);


		ImGui::End();

		ImGui::PopStyleVar();
	}

private:
	std::shared_ptr<Walnut::Image> m_Image;
	uint32_t* m_ImageData = nullptr;
	int m_RenderCurve = 0;

	size_t m_Index = 0;

	glm::vec2 m_Point{ 175. / 1600., 403. / 874. };

	Glyph m_glyph{ "..\\polices\\times.ttf", 'I' };

	SolveQuinticGPU solver;


	//Bezier m_Curve{ {{0.5, 0.5}, {0., 1.0}, {1., 1.}, {.5, .5 }}, 1000 };

};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "vulkan";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	return app;
}