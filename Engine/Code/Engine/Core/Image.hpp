#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"


class Image
{
public:
	Image();
	Image(char const* imageFilePath);
	Image(IntVec2 size, Rgba8 color);

	IntVec2	GetDimensions() const;
	std::string const& GetImageFilePath() const;
	const void* GetRawData() const;
	Rgba8 GetTexelColor(IntVec2 const& texelCoords) const;
	void SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor);
	void SetTexelColor(Vec2 const& texelUVs, Rgba8 const& newColor);

	void SetTexels(std::vector<Rgba8> const& rgbaTexels);
	void SetImageFilePath(std::string const& imageFilePath);
	void SetDimensions(IntVec2 const& dimensions);

private:
	std::string	m_imageFilePath;
	IntVec2	m_dimensions = IntVec2(0, 0);
	std::vector<Rgba8> m_rgbaTexels;  // or Rgba8* m_rgbaTexels = nullptr; if you prefer new[] and delete[]
};
