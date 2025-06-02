#pragma once
#include <map>
#include "Engine/Core/HCIS.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

struct Rgba8;
struct Vec2;
struct IntVec2;

class TypedPropertyBase
{
public:
	TypedPropertyBase() = default;
	virtual ~TypedPropertyBase() = default;
};

template<typename T>
class TypedProperty : public TypedPropertyBase
{
public:
	TypedProperty(T data);
	virtual ~TypedProperty() override;
	T m_data;
};

class NamedProperties
{
public:
	template<typename T> void SetValue(HCIS const& key, T const& value);

	template<typename T> T	GetValue(HCIS const& key, T const& defaultValue) const;

	void					SetValue(std::string const& keyName, char const* newValue);
	std::string				GetValue(std::string const& keyName, std::string const& defaultValue) const;
	bool					GetValue(std::string const& keyName, bool defaultValue) const;
	int						GetValue(std::string const& keyName, int defaultValue) const;
	unsigned int			GetValue(std::string const& keyName, unsigned int defaultValue) const;
	float					GetValue(std::string const& keyName, float defaultValue) const;
	std::string				GetValue(std::string const& keyName, char const* defaultValue) const;
	Rgba8					GetValue(std::string const& keyName, Rgba8 const& defaultValue) const;
	Vec2					GetValue(std::string const& keyName, Vec2 const& defaultValue) const;
	IntVec2					GetValue(std::string const& keyName, IntVec2 const& defaultValue) const;

private:
	std::map<HCIS, TypedPropertyBase*> m_keyValuePairs;
	std::string ExtractStringFromTypedProperty(TypedPropertyBase* property, std::string const& defaultValue) const;
};

template<typename T>
void NamedProperties::SetValue(HCIS const& key, T const& value)
{
	auto iter = m_keyValuePairs.find(key);
	if (iter != m_keyValuePairs.end())
	{
		TypedProperty<T>* property = dynamic_cast<TypedProperty<T>*>(iter->second);
		if (property != nullptr)
		{
			property->m_data = value;
		}
		else
		{
			delete iter->second;
			m_keyValuePairs[key] = new TypedProperty<T>(value);
		}
	}
	else
	{
		m_keyValuePairs[key] = new TypedProperty<T>(value);
	}
}

template<typename T>
T NamedProperties::GetValue(HCIS const& key, T const& defaultValue) const
{
	auto iter = m_keyValuePairs.find(key);
	if (iter != m_keyValuePairs.end())
	{
		TypedProperty<T>* property = dynamic_cast<TypedProperty<T>*>(iter->second);
		if (property != nullptr)
		{
			return property->m_data;
		}
		else
		{
			ERROR_AND_DIE(Stringf("Unexpected type in named property %s", key.c_str()));
		}

	}
	return defaultValue;
}

template<typename T>
TypedProperty<T>::TypedProperty(T data)
	: m_data(data)
{
}

template<typename T>
TypedProperty<T>::~TypedProperty()
{
}


