#include "Engine/Core/Image.hpp"

#define STB_IMAGE_IMPLEMENTATION // Exactly one .CPP (this Image.cpp) should #define this before #including stb_image.h
#include "ThirdParty/stb/stb_image.h"

Image::Image()
{
}

Image::Image(char const* imageFilePath)
	: m_imageFilePath(imageFilePath)
{
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	int componentsPerPixel; // 3 for RGB 4 for RGBA
	unsigned char *colorData = stbi_load(imageFilePath, &m_dimensions.x, &m_dimensions.y, &componentsPerPixel, 0);
	GUARANTEE_OR_DIE(colorData != nullptr, Stringf("File not found %s", imageFilePath));
	GUARANTEE_OR_DIE(componentsPerPixel == 3 || componentsPerPixel == 4, "Unsuported image format, must be RGB or RGBA");
	int texelsToReserve = m_dimensions.x * m_dimensions.y;
	m_rgbaTexels.reserve(texelsToReserve);
	for (int y = 0; y < m_dimensions.y; ++y)
	{
		for (int x = 0; x < m_dimensions.x; ++x)
		{
			int texelIdx = x + y * m_dimensions.x;
			int rIdx = texelIdx * componentsPerPixel;
			int gIdx = (texelIdx * componentsPerPixel) + 1;
			int bIdx = (texelIdx * componentsPerPixel) + 2;
			if (componentsPerPixel == 3)
			{
				m_rgbaTexels.push_back(Rgba8(colorData[rIdx], colorData[gIdx], colorData[bIdx], 255));
			}
			else if (componentsPerPixel == 4)
			{
				int aIdx = (texelIdx * componentsPerPixel) + 3;
				m_rgbaTexels.push_back(Rgba8(colorData[rIdx], colorData[gIdx], colorData[bIdx], colorData[aIdx]));
			}
		}
	}
	stbi_image_free(colorData);
}

Image::Image(IntVec2 size, Rgba8 color)
	: m_dimensions(size)
{
	m_rgbaTexels.reserve(m_dimensions.x * m_dimensions.y);
	for (int y = 0; y < m_dimensions.y; ++y)
	{
		for (int x = 0; x < m_dimensions.x; ++x)
		{
			m_rgbaTexels.push_back(color);
		}
	}
}

std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

const void* Image::GetRawData() const
{
	return (void*)m_rgbaTexels.data();
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

Rgba8 Image::GetTexelColor(IntVec2 const& texelCoords) const
{
	int texelIdx = texelCoords.x + texelCoords.y * m_dimensions.x;
	GUARANTEE_OR_DIE(texelIdx < (int)m_rgbaTexels.size(), "texel coords are out of bounds");
	return m_rgbaTexels[texelIdx];
}

void Image::SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor)
{
	int texelIdx = texelCoords.x + texelCoords.y * m_dimensions.x;
	GUARANTEE_OR_DIE(texelIdx < (int)m_rgbaTexels.size(), "texel coords are out of bounds");
	m_rgbaTexels[texelIdx] = newColor;
}

void Image::SetTexelColor(Vec2 const& texelUVs, Rgba8 const& newColor)
{
	IntVec2 texelCoords;

	texelCoords.x = (int)((float)m_dimensions.x * texelUVs.x);
	texelCoords.y = (int)((float)m_dimensions.y * texelUVs.y);
	SetTexelColor(texelCoords, newColor);
}

void Image::SetTexels(std::vector<Rgba8> const& rgbaTexels)
{
	m_rgbaTexels = rgbaTexels;
}

void Image::SetImageFilePath(std::string const& imageFilePath)
{
	m_imageFilePath = imageFilePath;
}

void Image::SetDimensions(IntVec2 const& dimensions)
{
	m_dimensions = dimensions;
}
