#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

Rgba8::Rgba8() {}

Rgba8 const Rgba8::WHITE(255, 255, 255, 255);
Rgba8 const Rgba8::RED(255, 0, 0, 255);
Rgba8 const Rgba8::GREEN(0, 255, 0, 255);
Rgba8 const Rgba8::BLUE(0, 0, 255, 255);
Rgba8 const Rgba8::MAGENTA(255, 0, 255, 255);
Rgba8 const Rgba8::CYAN(0, 255, 255, 255);
Rgba8 const Rgba8::YELLOW(255, 255, 0, 255);
Rgba8 const Rgba8::BLACK(0, 0, 0, 255);



Rgba8::Rgba8(unsigned char initialR, unsigned char initialG, unsigned char initialB, unsigned char initialA)
	: r(initialR)
	, g(initialG)
	, b(initialB)
	, a(initialA)
{}

Rgba8::Rgba8(unsigned char initialR, unsigned char initialG, unsigned char initialB)
	: r(initialR)
	, g(initialG)
	, b(initialB)
	, a(255)
{}

Rgba8 LerpColor(Rgba8 start, Rgba8 end, float fraction)
{
	float rNormal = Lerp(NormalizeByte(start.r), NormalizeByte(end.r), fraction);
	float gNormal = Lerp(NormalizeByte(start.g), NormalizeByte(end.g), fraction);
	float bNormal = Lerp(NormalizeByte(start.b), NormalizeByte(end.b), fraction);
	float aNormal = Lerp(NormalizeByte(start.a), NormalizeByte(end.a), fraction);

	unsigned char rByte = DenormalizeByte(rNormal);
	unsigned char gByte = DenormalizeByte(gNormal);
	unsigned char bByte = DenormalizeByte(bNormal);
	unsigned char aByte = DenormalizeByte(aNormal);

	return Rgba8(rByte, gByte, bByte, aByte);
}

void Rgba8::SetFromText(char const* text)
{
	Strings values = SplitStringOnDelimiter(text, ',');
	GUARANTEE_OR_DIE(values.size() == 3 || values.size() == 4, "Parsing Rgba8 failed to get rgb values");
	this->r = static_cast<unsigned char>(ClampInt(atoi(values[0].c_str()), 0, 255));
	this->g = static_cast<unsigned char>(ClampInt(atoi(values[1].c_str()), 0, 255));
	this->b = static_cast<unsigned char>(ClampInt(atoi(values[2].c_str()), 0, 255));
	if (values.size() == 4)
	{
		this->a = static_cast<unsigned char>(ClampInt(atoi(values[3].c_str()), 0, 255));
	}
	else
	{
		this->a = 255;
	}
}

void Rgba8::GetAsFloats(float* colorsAsFloats) const
{
	colorsAsFloats[0] = NormalizeByte(r);
	colorsAsFloats[1] = NormalizeByte(g);
	colorsAsFloats[2] = NormalizeByte(b);
	colorsAsFloats[3] = NormalizeByte(a);
}

void Rgba8::SetFromFloats(float* colorAsFloats)
{
	r = DenormalizeByte(colorAsFloats[0]);
	g = DenormalizeByte(colorAsFloats[1]);
	b = DenormalizeByte(colorAsFloats[2]);
	a = DenormalizeByte(colorAsFloats[3]);
}

bool Rgba8::operator==(Rgba8 const& compare) const
{
	return r == compare.r && g == compare.g && b == compare.b && a == compare.a;
}

