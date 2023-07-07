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

		{
			const Walnut::ScopedTimer timer{ "render time" };
			const auto& curves = m_glyph.m_Curves;

			if (curvesData.size() != curves.size())
				curvesData.resize(curves.size());

			for (size_t i = 0; i < curves.size(); i++)
			{
				const auto& curve = curves[i];
				const auto vectors = curve.getVectors();
				const auto bbox = curve.bbox();

				const glm::vec2 a = { bbox.x, bbox.y };
				const glm::vec2 b = { bbox.z, bbox.w };
				const glm::vec2 center = (a + b) / 2.f;

				CurvesData data{};
				data.P_0 = vectors[0];
				data.p1 = vectors[1];
				data.p2 = vectors[2];
				data.p3 = vectors[3];
				data.bisector = m_glyph.m_Bisectors[i];
				data.PointsCount = glm::uint(curve.size());
				data.centerAndTopRight = { center, b - center };

				curvesData[i] = data;

			}

			renderer.updateUBO(curvesData);
			renderer.renderSDF(width, height, curvesData.size());				
		}
		if (m_Switch == 0)
		{
			const std::vector<glm::vec4> result = renderer.getResult();

			for (size_t i = 0; i < width * height; i++)
				if(result[i].y <= 0)
					m_ImageData[i] = 0xff00ff00;
		}
		else if (m_Switch == 1)
		{
			//renderer.updateUBO(m_glyph.m_Angles);
			renderer.generateImage(m_glyph.m_Curves.size());
			const std::vector<OUTPUTIMAGE> image = renderer.getImage();

			for (size_t i = 0; i < width * height; i++)
				m_ImageData[i] = image[i].color;

		}

		//	// 172346
		//	// 149191
		//	// 110802
		//	// 1141258
		//}


		// 8259 264 172

		//for (const auto& outline : m_glyph.m_Contours)
		//	for(const auto& curve : m_glyph.m_Curves)
		//		for (float i = 0; i < 1.0; i += 0.001)
		//		{
		//			const glm::vec2 location = computeBezier(curve, i) * glm::vec2{width, height};
		//			drawSq(location, width, height, m_ImageData, 5, 0xff0000ff);
		//		}

		//const auto& curve = m_glyph.m_Curves[m_CurveIndex];
		//for (float i = 0; i < 1.0; i += 0.001)
		//{
		//	const glm::vec2 location = computeBezier(curve, i) * glm::vec2{width, height};
		//	drawSq(location, width, height, m_ImageData, 5, 0xff00ffff);
		//}


		for (size_t i = m_CurveIndex; i < m_CurveIndex + 1; i++)
		{
			const glm::vec4 point = m_glyph.m_Bisectors[i];
			const auto& curve = m_glyph.m_Curves[i];

			const glm::vec2 firstBisector = { point.x, point.y };
			const glm::vec2 secondBisector = { point.z, point.w };

			drawSq(firstBisector * 50.f + curve.getFirstPoint() * glm::vec2(width, height), width, height, m_ImageData, 21, 0xffffff00); //blue firt bisector
			drawSq(secondBisector * 50.f + curve.getLastPoint() * glm::vec2(width, height), width, height, m_ImageData, 21, 0xff00f0ff);

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

		ImGui::InputInt("Curve Index", (int*)&m_CurveIndex, 1, 100);
		//ImGui::InputInt("Index", (int*)&m_Index, 1, 1000);

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

	size_t m_CurveIndex = 0;
	size_t m_Index = 0;
	size_t m_Switch = 0;

	// PB with 9 HyliaSerifBeta-Regular.otf (parsing (bisectors) && rendering)
	// PB with J HyliaSerifBeta-Regular.otf (rendering tiny dot on a bisector)
	// PB with 5 HyliaSerifBeta-Regular.otf (parsing (bisectors))

	Glyph m_glyph{ "..\\polices\\HyliaSerifBeta-Regular.otf", '8' };

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