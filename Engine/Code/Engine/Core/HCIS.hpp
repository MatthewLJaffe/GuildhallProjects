#pragma once
#include "Engine/Core/StringUtils.hpp"

//short for HashedCaseInsensitiveString
class HCIS
{
public:
	HCIS() = default;
	HCIS(HCIS const& copyFrom);
	HCIS(char const* text);
	HCIS(std::string const& text);
	static unsigned int Hash(char const* text);
	unsigned int GetHash() const;
	std::string GetOriginalString() const;
	char const* c_str() const;
	bool operator<(HCIS const& compare) const;
	bool operator==(HCIS const& compare) const;
	bool operator!=(HCIS const& compare) const;
	bool operator==(char const* text) const;
	bool operator!=(char const* text) const;
	bool operator==(std::string const& text) const;
	bool operator!=(std::string const& text) const;
	void operator=(HCIS const& assignFrom);
	void operator=(char const* text);
	void operator=(std::string const& text);
private:
	std::string m_caseIntactText = "";
	unsigned int m_lowerCaseHash = 0;
};