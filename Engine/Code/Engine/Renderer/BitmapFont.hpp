#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


enum TextDrawMode
{
	SHRINK_TO_FIT,
	OVERRUN,
	WRAP,
	WRAP_WORDS
};
//------------------------------------------------------------------------------------------------
class BitmapFont
{
	friend class Renderer; // Only the Renderer can create new BitmapFont objects!

private:
	BitmapFont(char const* fontFilePathNameWithNoExtension, Texture* fontTexture);

public:
	Texture* GetTexture();

	void AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f, char charToIgnore = '#');

	void AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight,
		std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f,
		Vec2 const& alignment = Vec2(.5f, .5f), TextDrawMode mode = TextDrawMode::SHRINK_TO_FIT, int maxGlyphsToDraw = 99999, Vec2 padding = Vec2::ZERO, char paddingChar='#');

	void AddVertsForText3DAtOriginXForward(std::vector<Vertex_PCU>& verts,
		float cellHeight, std::string const& text,
		Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f,
		Vec2 const& alignment = Vec2(.5f, .5f), int maxGlyphsToDraw = 9999999);

	float GetTextWidth(float cellHeight, std::string const& text, float cellAspect = 1.f);

protected:
	float GetGlyphAspect(int glyphUnicode) const; // For now this will always return 1.0f!!!
	float GetTextLinesWidth(float cellHeight, Strings const& textLines, float cellAspect);
	void ShrinkToFit(Vec2 const& boxDimensions, float& out_CellHeight, float& out_TextLinesWidth, float& out_TextLinesHeight);
	Strings SplitStringByWrap(std::string stringToSplit, float boxWidth, float glyphWidth);
	Strings SplitStringByWrapWords(std::string stringToSplit, float boxWidth, float glyphWidth);
	int GetFurthestWhitespaceIndex(std::string const& str);

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet	m_fontGlyphsSpriteSheet;
};
