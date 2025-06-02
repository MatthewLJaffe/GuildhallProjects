#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture* fontTexture)
	: m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension)
	, m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
{ }

Texture* BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, char charToIgnore)
{
	for (size_t i = 0; i < text.length(); i++)
	{
		if (text[i] == charToIgnore)
		{
			continue;
		}
		SpriteDefinition charDefinition = m_fontGlyphsSpriteSheet.GetSpriteDef((int)text[i]);
		Vec2 charMins = textMins + Vec2::RIGHT * (cellHeight * cellAspect * (float)i);
		Vec2 charMaxs = charMins + Vec2(cellHeight * cellAspect, cellHeight);
		AABB2 charBounds(charMins, charMaxs);
		AABB2 uvs = charDefinition.GetUVs();
		AddVertsForAABB2D(vertexArray, charBounds, tint, uvs.m_mins, uvs.m_maxs);
	}
}

void BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, 
	Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextDrawMode mode, int maxGlyphsToDraw, Vec2 padding, char paddingChar)
{
	if (text == "")
	{
		return;
	}
	AABB2 paddedBox(box.m_mins + padding, box.m_maxs - padding);
	Strings textLines;

	if (mode == TextDrawMode::SHRINK_TO_FIT || mode == TextDrawMode::OVERRUN)
	{
		textLines = SplitStringOnDelimiter(text, '\n');
	}
	else if (mode == WRAP)
	{
		textLines = SplitStringByWrap(text, paddedBox.GetDimensions().x, cellHeight * cellAspect);
	}
	else if (mode == WRAP_WORDS)
	{
		textLines = SplitStringByWrapWords(text, paddedBox.GetDimensions().x, cellHeight * cellAspect);
	}

	Vec2 const& boxDimensions = paddedBox.GetDimensions();
	float textLinesHeight = static_cast<float>(textLines.size() * cellHeight);
	//textWidth is the width of the longestLine
	float textLinesWidth = GetTextLinesWidth(cellHeight, textLines, cellAspect);

	float adjustedCellHeight = cellHeight;
	if (mode == TextDrawMode::SHRINK_TO_FIT)
	{
		ShrinkToFit(boxDimensions, adjustedCellHeight, textLinesWidth, textLinesHeight);
	}

	//calculate left and top padding
	float totalPaddingX = boxDimensions.x - textLinesWidth;
	float leftPadding = Lerp(0.f, totalPaddingX, alignment.x);
	float totalPaddingY = boxDimensions.y - textLinesHeight;
	float bottomPadding = Lerp(0.f, totalPaddingY, alignment.y);
	float topPadding = totalPaddingY - bottomPadding;

	int remainingGlyphs = maxGlyphsToDraw;
	//add text verts for each line
	for (size_t i = 0; i < textLines.size(); i++)
	{
		float currLineOffset = static_cast<float>(i + 1) * adjustedCellHeight;
		Vec2 minsForLine = Vec2(paddedBox.m_mins.x + leftPadding, paddedBox.m_maxs.y - topPadding - currLineOffset);
		AddVertsForText2D(vertexArray, minsForLine, adjustedCellHeight, textLines[i].substr(0, remainingGlyphs), tint, cellAspect, paddingChar);

		//update remaining glyphs
		int lineLength = (int)textLines[i].length();
		remainingGlyphs -= lineLength;
		if (remainingGlyphs <= 0)
		{
			break;
		}
	}

}

void BitmapFont::AddVertsForText3DAtOriginXForward(std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, int maxGlyphsToDraw)
{
	UNUSED(maxGlyphsToDraw);
	float textWidth = (float)text.size() * cellAspect * cellHeight;
	Vec2 textMins(Lerp(-textWidth, 0.f, alignment.x), Lerp(-cellHeight, 0.f, alignment.y));
	AddVertsForText2D(verts, textMins, cellHeight, text, tint, cellAspect);
	Mat44 textOrientation;
	//textOrientation.AppendZRotation(180.f);
	textOrientation.AppendXRotation(90.f);
	TransformVertexArray3D(verts, textOrientation);
}

float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspect)
{
	float width = 0.f;
	for (size_t i = 0; i < text.size(); i++)
	{
		width += GetGlyphAspect((int)text[i]) * cellAspect * cellHeight;
	}
	return width;
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	return m_fontGlyphsSpriteSheet.GetSpriteDef(glyphUnicode).GetAspect();
}

float BitmapFont::GetTextLinesWidth(float cellHeight, Strings const& textLines, float cellAspect)
{
	float maxLineWidth = GetTextWidth(cellHeight, textLines[0], cellAspect);
	for (size_t i = 1; i < textLines.size(); i++)
	{
		float currLineWidth =  GetTextWidth(cellHeight, textLines[i], cellAspect);
		if (currLineWidth > maxLineWidth)
		{
			maxLineWidth = currLineWidth;
		}
	}
	return maxLineWidth;
}

void BitmapFont::ShrinkToFit(Vec2 const& boxDimensions, float& out_CellHeight, float& out_TextLinesWidth, float& out_TextLinesHeight)
{
	float widthScaling = boxDimensions.x / out_TextLinesWidth;
	float heightScaling = boxDimensions.y / out_TextLinesHeight;
	float scaleFactor = 1.f;
	if (scaleFactor > heightScaling)
	{
		scaleFactor = heightScaling;
	}
	if (scaleFactor > widthScaling)
	{
		scaleFactor = widthScaling;
	}
	out_CellHeight *= scaleFactor;
	out_TextLinesHeight *= scaleFactor;
	out_TextLinesWidth *= scaleFactor;
}

Strings BitmapFont::SplitStringByWrap(std::string stringToSplit, float boxWidth, float glyphWidth)
{
	Strings splitString;
	int stringsPerLine = (int)floorf(boxWidth / glyphWidth);
	for (int i = 0; i < (int)stringToSplit.size(); i += stringsPerLine)
	{
		int substrStart = i;
		int substrLength = ClampInt(stringsPerLine, 0, (int)stringToSplit.size() - substrStart);
		splitString.push_back(stringToSplit.substr(substrStart, substrLength));
	}

	return splitString;
}

Strings BitmapFont::SplitStringByWrapWords(std::string stringToSplit, float boxWidth, float glyphWidth)
{
	Strings splitString;
	int stringsPerLine = (int)floorf(boxWidth / glyphWidth);
	int i = 0;
	while (i < (int)stringToSplit.size())
	{
		int substrStart = i;
		int substrLength = ClampInt(stringsPerLine, 0, (int)stringToSplit.size() - substrStart);
		std::string wrappedLine = stringToSplit.substr(substrStart, substrLength);
		int furthestWhiteSpaceIdx = GetFurthestWhitespaceIndex(wrappedLine);
		if (furthestWhiteSpaceIdx != -1 && substrLength == stringsPerLine)
		{
			substrLength = furthestWhiteSpaceIdx;
			wrappedLine = wrappedLine.substr(0, substrLength);
			i += (substrLength + 1);
		}
		else
		{
			i += substrLength;
		}
		splitString.push_back(wrappedLine);
	}

	return splitString;
}

int BitmapFont::GetFurthestWhitespaceIndex(std::string const& str)
{
	int furthestWhitespaceIdx = -1;
	for (int i = 0; i < (int)str.size(); i++)
	{
		if (isspace(str[i]) != 0)
		{
			furthestWhitespaceIdx = i;;
		}
	}
	return furthestWhitespaceIdx;
}
