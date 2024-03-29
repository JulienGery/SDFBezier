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

#include "../TMP/render-sdf.h"
#include "../TMP/Bitmap.h"

#define RED 0x000000ff
#define GREEN 0x0000ff00
#define BLUE 0x00ff0000
#define ALPHA 0xff000000


uint32_t ToDelete(const float f)
{
	uint32_t ret;
	std::memcpy(&ret, &f, sizeof(float));
	return ret;
}

float getColor(const uint32_t color, const uint32_t mask)
{
	return color & mask;
}

uint32_t getColor(const float color[3])
{
	uint32_t ret = 0;

	if (color[0])
		ret += RED;
	if (color[1])
		ret += GREEN;
	if (color[2])
		ret += BLUE;
	
	return ret;
}

msdfgen::Bitmap<float, 3> convertMSDF(const std::vector<OUTPUTIMAGE>& MSDF, const size_t width, const size_t height)
{
	msdfgen::Bitmap ret = msdfgen::Bitmap<float, 3>(width, height);

	for (size_t j = 0; j < height; j++)
		for (size_t i = 0; i < width; i++)
		{
			const size_t index = j * width + i;

			const uint32_t color = ToDelete(MSDF[index].color.x);

			ret(i, j)[0] = getColor(color, RED);
			ret(i, j)[1] = getColor(color, GREEN);
			ret(i, j)[2] = getColor(color, BLUE);
		}

	return ret;
}

std::vector<uint32_t> convertMSDF(const msdfgen::Bitmap<float, 3>& MSDF)
{
	const size_t width = MSDF.width();
	const size_t height = MSDF.height();

	std::vector<uint32_t> ret(width * height);

	for (size_t j = 0; j < height; j++)
		for (size_t i = 0; i < width; i++)
		{
			const size_t index = j * width + i;
			ret[index] = getColor(MSDF(i, j));
		}

	return ret;
}

std::vector<uint32_t> convertMSDF(const msdfgen::Bitmap<float, 1>& MSDF)
{
	const size_t width = MSDF.width();
	const size_t height = MSDF.height();

	std::vector<uint32_t> ret(width * height);

	for (size_t j = 0; j < height; j++)
		for (size_t i = 0; i < width; i++)
		{
			const size_t index = j * width + i;
			if (MSDF(i, j)[0])
				ret[index] = ALPHA | RED | GREEN | BLUE;
			else
				ret[index] = 0x0;
		}

	return ret;
}


std::vector<uint32_t> tmpRenderSdf(const std::vector<OUTPUTIMAGE>& MSDF, const size_t msdfWidth, const size_t msdfHeight, const size_t targetWidth, const size_t targetHeight, const glm::vec2 scale)
{
	const double avgScale = 1;
	const double range = .5;

	msdfgen::Bitmap<float, 1> render(targetWidth, targetHeight);
	const msdfgen::Bitmap<float, 3> msdf = convertMSDF(MSDF, msdfWidth, msdfHeight);
	msdfgen::renderSDF(render, msdf, avgScale * range, .5f);


	std::vector<uint32_t> ret = convertMSDF(render);

	return ret;
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
			renderer.renderSDF(m_MSDFWidth, m_MSDFHeight, curvesData.size());
			//renderer.renderSDF(width, height, curvesData.size());
		}


		if (m_Switch == 0)
		{
			const std::vector<glm::vec4> result = renderer.getResult();

			for (size_t i = 0; i < result.size(); i++)
				if(result[i].y <= 0)
					m_ImageData[i] = 0xff00ff00;
		}
		else if (m_Switch == 1 || m_Switch == 2)
		{
			std::vector<GenerateImage> vec(0);
			for (const auto& outline : m_glyph.m_Contours)
			{
				const auto& curves = outline.m_Curves;
				for (int i = 0; i < outline.m_Curves.size(); i++) 
				{
					const auto& curve = outline.m_Curves[i];

					const glm::vec2 startCurve = curve(0.);
					const glm::vec2 endCurve = curve(1.0);

					const size_t nextCurveIndex = (i + 1) % curves.size();
					const size_t previusCurveIndex = i == 0 ? curves.size() - 1 : i - 1;

					const glm::vec2 previusCurveEndDerivate = curves[previusCurveIndex].endDerivate();
					const glm::vec2 nextCurveStartDerivate = curves[nextCurveIndex].startDerivate();

					GenerateImage generateImage{};
					generateImage.startAndEndPoint = { startCurve, endCurve };
					generateImage.previusCurveEndDerivateAndNextcurveStartDerivate = { glm::normalize(previusCurveEndDerivate), glm::normalize(nextCurveStartDerivate) };
					generateImage.colorAndAngles = { curve.m_Color, outline.m_Angles[i], 0};
					
					vec.push_back(generateImage);
				}
			}

			renderer.updateUBO(vec);
			renderer.generateImage(m_glyph.m_Curves.size());
			const std::vector<OUTPUTIMAGE> image = renderer.getImage();

			if (m_Switch == 1)
			{
				for (size_t i = 0; i < image.size(); i++)
					m_ImageData[i] = ToDelete(image[i].color.x);
			}

			else
			{
				const std::vector<uint32_t> tmpSDF = tmpRenderSdf(image, m_MSDFWidth, m_MSDFHeight, width, height, { 1.0, 1.0 });

				for (size_t i = 0; i < width * height; i++)
					m_ImageData[i] = tmpSDF[i] | 0xff000000;
			}
		}


		/*const auto& curve = m_glyph.m_Curves[m_CurveIndex];
		for (float i = 0; i < 1.0; i += 0.001)
		{
			const glm::vec2 location = curve(i) * glm::vec2{width, height};
			drawSq(location, width, height, m_ImageData, 5, 0xffdb9c74);
		}*/


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

		ImGui::InputInt("msdf width", (int*)&m_MSDFWidth, 1, 100);
		ImGui::InputInt("msdf height", (int*)&m_MSDFHeight, 1, 100);


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

	size_t m_MSDFWidth = 32;
	size_t m_MSDFHeight = 32;

	Glyph m_glyph{ "..\\polices\\times.ttf", 'A' };

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