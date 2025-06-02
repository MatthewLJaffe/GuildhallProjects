
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	for (XmlAttribute const* attr = element.FirstAttribute(); attr != nullptr; attr = attr->Next())
	{
		m_keyValuePairs[ToLower(std::string(attr->Name()))] = std::string(attr->Value());
	}
}

void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{

	m_keyValuePairs[ToLower(keyName)] = newValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter != m_keyValuePairs.end())
	{
		return iter->second;
	}
	return defaultValue;
}

bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	if (iter->second == "true")
	{
		return true;
	}

	return false;
}

int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	return atoi(iter->second.c_str());

}

unsigned int NamedStrings::GetValue(std::string const& keyName, unsigned int defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	return (unsigned int)strtoul(iter->second.c_str(), nullptr, 0);
}

float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	float valueAsFloat =static_cast<float>(atof(iter->second.c_str()));
	if (valueAsFloat == 0.f)
	{
		if (iter->second[0] == '0')
		{
			return 0.f;
		}
		return defaultValue;
	}
	return static_cast<float>(valueAsFloat);
}

std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	return iter->second;
}

Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	Rgba8 output = defaultValue;
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return output;
	}
	output.SetFromText(iter->second.c_str());
	return output;
}

Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	Vec2 output = defaultValue;
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return output;
	}
	output.SetFromText(iter->second.c_str());
	return output;
}

IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	IntVec2 output = defaultValue;
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return output;
	}
	output.SetFromText(iter->second.c_str());
	return output;
}
