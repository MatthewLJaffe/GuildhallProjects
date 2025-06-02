#pragma once
#include "Engine/Core/XmlUtils.hpp"

class Definition
{
public:
	virtual bool LoadFromXmlElement(XmlElement const& element) = 0;
	std::string m_name;
};