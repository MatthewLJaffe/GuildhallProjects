#include "Engine/Core/HCIS.hpp"

HCIS::HCIS(HCIS const& copyFrom)
	: m_caseIntactText(copyFrom.GetOriginalString())
	, m_lowerCaseHash(copyFrom.GetHash())
{

}

HCIS::HCIS(char const* text)
	: m_caseIntactText(text)
	, m_lowerCaseHash(Hash(ToLower(m_caseIntactText).c_str()))
{
}

HCIS::HCIS(std::string const& text)
	: m_caseIntactText(text)
	, m_lowerCaseHash(Hash(ToLower(m_caseIntactText).c_str()))
{
}

unsigned int HCIS::Hash(char const* text)
{
	unsigned int hash = 0;
	for (char const* scan = text; *scan != '\0'; scan++)
	{
		hash *= 31;
		hash += (unsigned int)*scan;
	}
	return hash;
}

unsigned int HCIS::GetHash() const
{
	return m_lowerCaseHash;
}

std::string HCIS::GetOriginalString() const
{
	return m_caseIntactText;
}

char const* HCIS::c_str() const
{
	return m_caseIntactText.c_str();
}

bool HCIS::operator<(HCIS const& compare) const
{
	return m_lowerCaseHash < compare.GetHash();
}

bool HCIS::operator==(HCIS const& compare) const
{
	if (m_lowerCaseHash != compare.GetHash())
	{
		return false;
	}
	return _stricmp(m_caseIntactText.c_str(), compare.c_str()) == 0;
}

bool HCIS::operator!=(HCIS const& compare) const
{
	if (m_lowerCaseHash != compare.GetHash())
	{
		return true;
	}
	return _stricmp(m_caseIntactText.c_str(), compare.c_str()) != 0;
}

bool HCIS::operator==(char const* text) const
{
	return _stricmp(c_str(), text) == 0;
}

bool HCIS::operator!=(char const* text) const
{
	return _stricmp(c_str(), text) != 0;
}

bool HCIS::operator==(std::string const& text) const
{
	return _stricmp(c_str(), text.c_str()) == 0;
}

bool HCIS::operator!=(std::string const& text) const
{
	return _stricmp(c_str(), text.c_str()) != 0;
}

void HCIS::operator=(HCIS const& assignFrom)
{
	m_caseIntactText = assignFrom.GetOriginalString();
	m_lowerCaseHash = assignFrom.GetHash();
}

void HCIS::operator=(char const* text)
{
	m_caseIntactText = text;
	m_lowerCaseHash = Hash(text);
}

void HCIS::operator=(std::string const& text)
{
	m_caseIntactText = text;
	m_lowerCaseHash = Hash(text.c_str());
}

