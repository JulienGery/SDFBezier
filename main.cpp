#include "Renderer.h"
#include "Glyph.h"

#include "render-sdf.h"
#include "Bitmap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <chrono>
#include <iostream>
#include <cstring>

#define RED 0x000000ff
#define GREEN 0x0000ff00
#define BLUE 0x00ff0000
#define CHANNEL_NUM 3


uint32_t ToDelete(const float f)
{
	uint32_t ret;
	std::memcpy(&ret, &f, sizeof(float));
	return ret;
}


template<typename T>
void customSwap(std::vector<T>& vec, const size_t width, const size_t height)
{
    for (size_t i = 0; i < (height / 2 * width); i++)
    {
        const uint32_t y = height - i / width;
        std::swap(vec[i], vec[y * width - width + (i % width)]);
    }
}
float getColor(const uint32_t color, const uint32_t mask)
{
	return color & mask;
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

msdfgen::Bitmap<float, 1> renderMSDF(const size_t targetWidth, const size_t targetHeight, const std::vector<OUTPUTIMAGE>& MSDF, const size_t msdfWidth, const size_t msdfHeight)
{
    const double avgScale = 1;
    const double range = 0.0244086;

    msdfgen::Bitmap<float, 1> render(targetWidth, targetHeight);
    const msdfgen::Bitmap<float, 3> msdf = convertMSDF(MSDF, msdfWidth, msdfHeight);
    msdfgen::renderSDF(render, msdf, .35e-02, -0.5);

    return render;
}

int main()
{
    Renderer renderer{};

    Glyph glyph{"../polices/HyliaSerifBeta-Regular.otf", '8'};

    const size_t width = 1920;
    const size_t height = 1080;

    const auto& curves = glyph.m_Curves;
    std::vector<CurvesData> curvesData(curves.size());

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
        data.bisector = glyph.m_Bisectors[i];
        data.PointsCount = curve.size();
        data.centerAndTopRight = { center, b - center };

        curvesData[i] = data;
    }

    const auto start = std::chrono::high_resolution_clock::now();

    renderer.updateUBO(curvesData);
    renderer.renderSDF(width, height, curvesData.size());

    const auto end = std::chrono::high_resolution_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "delta: " << delta.count() << " ms\n";

    std::vector<GenerateImage> vec(0);
    for (const auto& outline : glyph.m_Contours)
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
    renderer.generateImage(glyph.m_Curves.size());
    std::vector<OUTPUTIMAGE> tmpImage = renderer.getImage();

    customSwap(tmpImage, width, height);

    std::vector<uint32_t> image(tmpImage.size());
    for(size_t i = 0; i < image.size(); i++)
        image[i] = ToDelete(tmpImage[i].color.x);


    uint8_t* pixels = new uint8_t[width * height * CHANNEL_NUM];

    size_t index = 0;
    for(size_t i = 0; i < width * height; i++)
    {
        const int r = (image[i] & RED);
        const int g = (image[i] & GREEN) >> (4 * 2);
        const int b = (image[i] & BLUE) >> (4 * 4);

        pixels[index++] = r;
        pixels[index++] = g;
        pixels[index++] = b;
    }

    stbi_write_png("output.png", width, height, CHANNEL_NUM, pixels, width * CHANNEL_NUM);

    delete [] pixels;

    const size_t targetWidth = 1920;
    const size_t targetHeight = 1080;


    const msdfgen::Bitmap<float, 1> renderedMSDF = renderMSDF(targetWidth, targetHeight, tmpImage, width, height);


    pixels = new uint8_t[targetWidth * targetHeight * CHANNEL_NUM];
    index = 0;
    for(size_t i = 0; i < targetWidth * targetHeight; i++)
    {
        pixels[index++] = renderedMSDF[i] * 255;
        pixels[index++] = renderedMSDF[i] * 255;
        pixels[index++] = renderedMSDF[i] * 255;
    }


    stbi_write_png("render.png", targetWidth, targetHeight, CHANNEL_NUM, pixels, targetWidth * CHANNEL_NUM);
    delete [] pixels;
}
