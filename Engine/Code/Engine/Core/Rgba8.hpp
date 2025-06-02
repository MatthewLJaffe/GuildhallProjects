#pragma once

struct Rgba8
{
public:
	static const Rgba8 WHITE;
	static const Rgba8 RED;
	static const Rgba8 GREEN;
	static const Rgba8 BLUE;
	static const Rgba8 MAGENTA;
	static const Rgba8 CYAN;
	static const Rgba8 YELLOW;
	static const Rgba8 BLACK;

	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

	Rgba8();
	explicit Rgba8(unsigned char initialR, unsigned char initialG, unsigned char initialB, unsigned char initialA);
	Rgba8(unsigned char initialR, unsigned char initialG, unsigned char initialB);
	void SetFromText(char const* text);
	void GetAsFloats(float* colorsAsFloats) const;
	void SetFromFloats(float* colorsAsFloats);
	bool			operator==(Rgba8 const& compare) const;

};

Rgba8 LerpColor(Rgba8 start, Rgba8 end, float fraction);
