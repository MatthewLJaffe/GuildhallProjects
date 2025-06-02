#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB2.hpp"

int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	return element.IntAttribute(attributeName, defaultValue);
}

char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		return attributeValue[0];
	}
	return defaultValue;
}

bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	return element.BoolAttribute(attributeName, defaultValue);
}

float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	return element.FloatAttribute(attributeName, defaultValue);
}

Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	Rgba8 elementValue = defaultValue;
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue && std::string(attributeValue) != std::string(""))
	{
		elementValue.SetFromText(attributeValue);
		return elementValue;
	}
	return defaultValue;
}

Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	Vec2 elementValue = defaultValue;
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		elementValue.SetFromText(attributeValue);
		return elementValue;
	}
	return defaultValue;
}

FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const& defaultValue)
{
	FloatRange elementValue = defaultValue;
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		elementValue.SetFromText(attributeValue);
		return elementValue;
	}
	return defaultValue;
}

Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	Vec3 elementValue = defaultValue;
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		elementValue.SetFromText(attributeValue);
		return elementValue;
	}
	return defaultValue;
}

EulerAngles ParseXmlAttribute(XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue)
{
	EulerAngles elementValue = defaultValue;
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		elementValue.SetFromText(attributeValue);
		return elementValue;
	}
	return defaultValue;
}

IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	IntVec2 elementValue = defaultValue;
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		elementValue.SetFromText(attributeValue);
		return elementValue;
	}
	return defaultValue;
}

AABB2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, AABB2 const& defaultValue)
{
	AABB2 elementValue = defaultValue;
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		elementValue.SetFromText(attributeValue);
		return elementValue;
	}
	return defaultValue;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		return std::string(attributeValue);
	}
	return defaultValue;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		return std::string(attributeValue);
	}
	return std::string(defaultValue);
	
}

Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		return SplitStringOnDelimiter( std::string(attributeValue), ',');
	}
	return defaultValue;
}
