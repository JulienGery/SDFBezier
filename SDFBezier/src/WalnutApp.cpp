#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Walnut/Image.h"
#include "glm/vec2.hpp"
#include "glm/mat2x2.hpp"

#include "Glyph.h"
#include "SolveQuinticGPU.h"

#include "../outdated/Draw.h"

float inline crossProduct(const glm::vec2& d, const glm::vec2& v)
{
	return d.x * v.y - d.y * v.x;
}

glm::mat2 rotationMatrix(const float o)
{
	return {
		glm::cos(o), glm::sin(o),
		-glm::sin(o),  glm::cos(o)
	};
}


glm::vec2 bisector(const glm::vec2 a, const glm::vec2 b)
{
	const glm::vec2 result = { 1, 0 };

	const glm::vec2 x = glm::normalize(a);
	const glm::vec2 y = glm::normalize(b);
	
	const float o = glm::acos(glm::dot(x, y)) / 2.0;

	const auto matrix = rotationMatrix(o);
	
	return matrix * x;
}

class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer()
	{
		
		m_ImageData = new uint32_t[700 * 700];
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

		std::vector<glm::vec4> bistectors{};

		for (size_t i = 0; i < width * height; i++)
			m_ImageData[i] = 0x00000000;
		
		{
			const Walnut::ScopedTimer timer{ "render time" };
			const auto& curves = m_glyph.m_Curves;

			for (size_t i = 0; i < curves.size(); i++)
			{
				
				const auto jsp = curves[i].getVectors();
				solver.P_0 = jsp[0];
				solver.p1 = jsp[1];
				solver.p2 = jsp[2];
				solver.p3 = jsp[3];
				solver.m_Width = width;
				solver.m_Height = height;
				solver.m_CurveIndex = i;
				
				solver.m_Bis = {
						bisector(-curves[(i - 1) % curves.size()].getDerivateEnd(), curves[i].getStartDerivate()),
						bisector(-curves[i].getDerivateEnd(), curves[(i + 1) % curves.size()].getStartDerivate())
				};

				bistectors.push_back(solver.m_Bis);
			
				const size_t index = curves[i].size();
				solver.recordComputeCommandBuffers(index);
				solver.execute(index);
			}
				
		}

		const std::vector<glm::vec4> result = solver.getResult();

		for (size_t i = 0; i < result.size(); i++)
		{
			//if (result[i].w == m_Index && result[i].x < m_distance * m_distance)
				//m_ImageData[i] = 0xff'ff'ff'ff;
			if (result[i].w == m_Index)
				m_ImageData[i] = 0xff'00'00'ff;

			//else if (result[i].y < 0 && result[i].x < m_distance * m_distance)
				//m_ImageData[i] = 0xff'ff'ff'00;

			else if (result[i].y < 0)
				m_ImageData[i] = 0xff'00'ff'00;

			//if (result[i].x < m_distance * m_distance)
				//m_ImageData[i] = 0xff'ff'00'00;

			//m_ImageData[i] = 0xff'00'00'00 | (*(uint32_t*)&result[i].w);
		}

		//for (size_t i = 0; i < bistectors.size(); i++)
		{
			const size_t i = m_Index;

			const glm::vec4 point = bistectors[i];
			const auto& curve = m_glyph.m_Curves[i];

			const glm::vec2 firstBisector = { point.x, point.y };
			const glm::vec2 secondBisector = { point.z, point.w };

			drawSq(firstBisector * 50.f + curve.getFirstPoint(), width, height, m_ImageData, 21, 0xffff00ff);
			drawSq(secondBisector * 50.f + curve.getLastPoint(), width, height, m_ImageData, 21, 0xffffffff);

			drawSq(curve.getFirstPoint(), width, height, m_ImageData, 21, 0xffff00ff);
			drawSq(curve.getLastPoint(), width, height, m_ImageData, 21, 0xffffffff);


			//const auto point = m_glyph.m_Curves[m_Index].m_Points[i];
			//drawSq(point, width, height, m_ImageData, 15, 0xff00ffff);
		}

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

		/*const size_t pointCount = m_Curve.getSize();

		ImGui::InputFloat("p0x", &m_Curve.m_Points[0].x, 0.01f, 1.f, "%.2f");
		ImGui::InputFloat("p0y", &m_Curve.m_Points[0].y, 0.01f, 1.f, "%.2f");

		ImGui::InputFloat("p1x", &m_Curve.m_Points[1].x, 0.01f, 1.f, "%.2f");
		ImGui::InputFloat("p1y", &m_Curve.m_Points[1].y, 0.01f, 1.f, "%.2f");

		if (pointCount >= 3)
		{
			ImGui::InputFloat("p2x", &m_Curve.m_Points[2].x, 0.01f, 1.f, "%.2f");
			ImGui::InputFloat("p2y", &m_Curve.m_Points[2].y, 0.01f, 1.f, "%.2f");
		}

		if (pointCount == 4)
		{
			ImGui::InputFloat("p3x", &m_Curve.m_Points[3].x, 0.01f, 1.f, "%.2f");
			ImGui::InputFloat("p3y", &m_Curve.m_Points[3].y, 0.01f, 1.f, "%.2f");
		}*/

		ImGui::InputInt("Index", (int*)&m_Index, 1, 100);

		//ImGui::InputFloat("px", &m_Point.x, 0.01f, 1.f, "%.2f");
		//ImGui::InputFloat("py", &m_Point.y, 0.01f, 1.f, "%.2f");

		ImGui::InputInt("render curve", &m_RenderCurve, 0, 1);

		ImGui::InputFloat("distance", &m_distance, 0.01f, 1.f, "%.2f");

		ImGui::End();

		ImGui::PopStyleVar();
	}

private:
	std::shared_ptr<Walnut::Image> m_Image;
	uint32_t* m_ImageData = nullptr;
	int m_RenderCurve = 0;

	float m_distance = 2.5;

	size_t m_Index = 0;

	Glyph m_glyph{ "..\\polices\\times.ttf", 'u' };

	SolveQuinticGPU solver;

	//Bezier m_Curve{ {{0, 0}, {1, 1}} };

};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "vulkan";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	return app;
}