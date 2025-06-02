#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

void NamedProperties::SetValue(std::string const& keyName, char const* newValue)
{
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end())
	{
		TypedProperty<std::string>* property = dynamic_cast<TypedProperty<std::string>*>(iter->second);
		if (property != nullptr)
		{
			property->m_data = newValue;
		}
		else
		{
			delete iter->second;
			m_keyValuePairs[keyName] = new TypedProperty<std::string>(newValue);
		}
	}
	else
	{
		m_keyValuePairs[keyName] = new TypedProperty<std::string>(newValue);
	}
}


std::string NamedProperties::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end())
	{
		TypedPropertyBase* property = iter->second;
		return ExtractStringFromTypedProperty(property, defaultValue);
	}
	return defaultValue;
}

bool NamedProperties::GetValue(std::string const& keyName, bool defaultValue) const
{
	auto iter = m_keyValuePairs.find(keyName);
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	TypedPropertyBase* property = iter->second;
	std::string valueAsStr = ExtractStringFromTypedProperty(property, "Missing");
	if (valueAsStr == "Missing")
	{
		return GetValue<bool>(keyName, defaultValue);
	}
	if (valueAsStr == "true")
	{
		return true;
	}
	if (valueAsStr == "false")
	{
		return false;
	}
	return defaultValue;
}

int NamedProperties::GetValue(std::string const& keyName, int defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	TypedPropertyBase* property = iter->second;
	std::string valueAsStr = ExtractStringFromTypedProperty(property, "Missing");
	if (valueAsStr == "Missing")
	{
		return GetValue<int>(keyName, defaultValue);
	}
	return atoi(valueAsStr.c_str());

}

unsigned int NamedProperties::GetValue(std::string const& keyName, unsigned int defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	TypedPropertyBase* property = iter->second;
	std::string valueAsStr = ExtractStringFromTypedProperty(property, "Missing");
	if (valueAsStr == "Missing")
	{
		return GetValue<unsigned int>(keyName, defaultValue);
	}
	return (unsigned int)strtoul(valueAsStr.c_str(), nullptr, 0);
}

float NamedProperties::GetValue(std::string const& keyName, float defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	TypedPropertyBase* property = iter->second;
	std::string valueAsStr = ExtractStringFromTypedProperty(property, "Missing");
	if (valueAsStr == "Missing")
	{
		return GetValue<float>(keyName, defaultValue);
	}
	float valueAsFloat = static_cast<float>(atof(valueAsStr.c_str()));
	if (valueAsFloat == 0.f)
	{
		if (valueAsStr[0] == '0')
		{
			return 0.f;
		}
	}
	return static_cast<float>(valueAsFloat);
}

std::string NamedProperties::GetValue(std::string const& keyName, char const* defaultValue) const
{
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	TypedPropertyBase* property = iter->second;
	std::string valueAsStr = ExtractStringFromTypedProperty(property, defaultValue);
	return valueAsStr;
}

Rgba8 NamedProperties::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	Rgba8 output = defaultValue;
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return output;
	}

	TypedPropertyBase* property = iter->second;
	std::string valueAsStr = ExtractStringFromTypedProperty(property, "Missing");
	if (valueAsStr == "Missing")
	{
		return GetValue<Rgba8>(keyName, defaultValue);
	}
	output.SetFromText(valueAsStr.c_str());
	return output;
}

Vec2 NamedProperties::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	Vec2 output = defaultValue;
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return output;
	}

	TypedPropertyBase* property = iter->second;
	std::string valueAsStr = ExtractStringFromTypedProperty(property, "Missing");
	if (valueAsStr == "Missing")
	{
		return GetValue<Vec2>(keyName, defaultValue);
	}
	output.SetFromText(valueAsStr.c_str());
	return output;
}

IntVec2 NamedProperties::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	IntVec2 output = defaultValue;
	auto iter = m_keyValuePairs.find(ToLower(keyName));
	if (iter == m_keyValuePairs.end())
	{
		return output;
	}
	TypedPropertyBase* property = iter->second;
	std::string valueAsStr = ExtractStringFromTypedProperty(property, "Missing");
	if (valueAsStr == "Missing")
	{
		return GetValue<IntVec2>(keyName, defaultValue);
	}
	output.SetFromText(valueAsStr.c_str());
	return output;
}

std::string NamedProperties::ExtractStringFromTypedProperty(TypedPropertyBase* property, std::string const& defaultValue) const
{
	TypedProperty<std::string>* propertyStr = dynamic_cast<TypedProperty<std::string>*>(property);
	if (propertyStr != nullptr)
	{
		return propertyStr->m_data;
	}
	else
	{
		TypedProperty<char const*>* propertyCStr = dynamic_cast<TypedProperty<char const*>*>(property);
		if (propertyCStr != nullptr)
		{
			return propertyCStr->m_data;
		}
		else
		{
			//not an error?
			return defaultValue;
		}
	}
}
