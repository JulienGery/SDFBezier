#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Walnut/Image.h"
#include "glm/vec2.hpp"
#include "glm/mat2x2.hpp"

#include "Glyph.h"
#include "Renderer.h"

#include "../outdated/Draw.h"
#include <complex>



glm::vec2 computeBezier(const Bezier& bezier, const float t) // TMP
{
	const auto& vectors = bezier.getVectors();
	if (bezier.size() == 2)
		return vectors[0] + t * vectors[1];
	if (bezier.size() == 3)
		return vectors[0] + 2.0f * t * vectors[1] + t * t * vectors[2];
	return vectors[0] + 3.0f * t * vectors[1] + 
		3.0f * t * t * vectors[2] + t * t * t * vectors[3];
}

class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer()
	{
		
		m_ImageData = new uint32_t[700 * 700];
		m_Image = std::make_shared<Walnut::Image>(700, 700, Walnut::ImageFormat::RGBA, m_ImageData);
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


		for (const auto& outline : m_glyph.m_Outlines)
			for(const auto& curve : m_glyph.m_Curves)
				for (float i = 0; i < 1.0; i += 0.001)
				{
					const glm::vec2 location = computeBezier(curve, i) * glm::vec2{width, height};
					drawSq(location, width, height, m_ImageData, 5, 0xff0000ff);
				}

		const auto& curve = m_glyph.m_Curves[m_Index];
		for (float i = 0; i < 1.0; i += 0.001)
		{
			const glm::vec2 location = computeBezier(curve, i) * glm::vec2{width, height};
			drawSq(location, width, height, m_ImageData, 5, 0xff00ffff);
		}

		{
			const Walnut::ScopedTimer timer{ "render time" };
			const auto& curves = m_glyph.m_Curves;

			if (curvesData.size() != curves.size())
				curvesData.resize(curves.size());

			for (size_t i = 0; i < curves.size(); i++)
			{
				const auto& curve = curves[i];
				const auto vectors = curve.getVectors();
				
				CurvesData data{};
				data.P_0 = vectors[0];
				data.p1 = vectors[1];
				data.p2 = vectors[2];
				data.p3 = vectors[3];
				data.bisector = m_glyph.m_Bisectors[i];
				data.PointsCount = glm::uint(curve.size());

				curvesData[i] = data;
			}

			renderer.updateUBO(curvesData);
			renderer.renderSDF(width, height, curvesData.size());				
		}
		if (m_Switch == 0)
		{
			const std::vector<glm::vec4> result = renderer.getResult();

			//std::cout << result[110802].x << ' ' << result[110802].y << ' ' << result[110802].z << ' ' << result[110802].w << '\n';


			for (size_t i = 0; i < width * height; i++)
				if(result[i].y <= 0)
					m_ImageData[i] = 0xff00ff00;
		}
		//else if (m_Switch == 1)
		//{
		//	//renderer.updateUBO(m_glyph.m_Angles);
		//	renderer.generateImage();
		//	const std::vector<OUTPUTIMAGE> image = renderer.getImage();

		//	for (size_t i = 0; i < width * height; i++)
		//		m_ImageData[i] = image[i].color;

		//	//for(size_t i = width; i < width * height - width; i++)
		//	//	if (image[i - width].color == 0xffffffff &&
		//	//		image[i + width].color == 0xffffffff &&
		//	//		image[i - 1].color == 0xffffffff &&
		//	//		image[i + 1].color == 0xffffffff &&
		//	//		image[i].color == 0xff0000ff)
		//	//	{
		//	//		std::cout << i << '\n';
		//	//		break;
		//	//	}


		//	// 172346
		//	// 149191
		//	// 110802
		//}

		////m_ImageData[110802] = 0xff00f0ff;


		for (size_t i = m_Index; i < m_Index + 1; i++)
		{
			const glm::vec4 point = m_glyph.m_Bisectors[i];
			const auto& curve = m_glyph.m_Curves[i];

			const glm::vec2 firstBisector = { point.x, point.y };
			const glm::vec2 secondBisector = { point.z, point.w };

			
			if (curve.size() == 4)
			{
				drawSq(-glm::normalize(curve.startDerivate()) * 50.f + curve.getFirstPoint() * glm::vec2(width, height), width, height, m_ImageData, 21, 0xffffff00);
				drawSq(glm::normalize(curve.endDerivate()) * 50.f + curve.getLastPoint() * glm::vec2(width, height), width, height, m_ImageData, 21, 0xff00f0ff);
			}
			else
			{
				drawSq(firstBisector * 50.f + curve.getFirstPoint() * glm::vec2(width, height), width, height, m_ImageData, 21, 0xffffff00); //blue firt bisector
				drawSq(secondBisector * 50.f + curve.getLastPoint() * glm::vec2(width, height), width, height, m_ImageData, 21, 0xff00f0ff);
			}

			drawSq(curve.getFirstPoint() * glm::vec2(width, height), width, height, m_ImageData, 21, 0xffff0000);
			drawSq(curve.getLastPoint() * glm::vec2(width, height), width, height, m_ImageData, 21, 0xff0000ff);
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

		ImGui::InputInt("render curve", &m_RenderCurve, 1, 1);
		ImGui::InputInt("Switch", (int*)&m_Switch, 1, 1);

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
	size_t m_Switch = 0;

	Glyph m_glyph{ "..\\polices\\HyliaSerifBeta-Regular.otf", 'S' };

	std::vector<CurvesData> curvesData;
	
	Renderer renderer;

};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "vulkan";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	return app;
}