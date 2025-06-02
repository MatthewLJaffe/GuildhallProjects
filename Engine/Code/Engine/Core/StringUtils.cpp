#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn, bool removeEmpty)
{
	Strings splitStrings;
	int substrStartIdx = 0;
	for (int i = 0; i < static_cast<int>( originalString.length() ); i++)
	{
		if (originalString[i] == delimiterToSplitOn)
		{
			int substringSize = i - substrStartIdx;
			if (substringSize == 0 && removeEmpty)
			{
				substrStartIdx++;
				continue;
			}
			splitStrings.push_back(std::string(originalString, substrStartIdx, substringSize));
			substrStartIdx = i + 1;
		}
	}
	if (!removeEmpty || (substrStartIdx <= (int)originalString.size() - 1 && substrStartIdx > 0))
	{
		splitStrings.push_back(std::string(originalString, substrStartIdx));
	}
	return splitStrings;
}

Strings SplitStringOnDelimiter(std::string const& originalString, std::string const& delimiterToSplitOn, bool removeEmpty)
{
	Strings splitStrings;
	int substrStartIdx = 0;
	for (int i = 0; i < static_cast<int>(originalString.length()); i++)
	{
		if (i + (int)delimiterToSplitOn.size() > (int)originalString.size())
		{
			break;
		}
		if (originalString[i] == delimiterToSplitOn[0])
		{
			for (int sIdx = 0; sIdx < (int)delimiterToSplitOn.size(); sIdx++)
			{
				if (originalString[i + sIdx] != delimiterToSplitOn[sIdx])
				{
					continue;
				}
			}
			int substringSize = i - substrStartIdx;
			if (substringSize == 0 && removeEmpty)
			{
				substrStartIdx += (int)delimiterToSplitOn.size();
				continue;
			}
			splitStrings.push_back(std::string(originalString, substrStartIdx, substringSize));
			substrStartIdx = i + (int)delimiterToSplitOn.size();
		}
	}
	if (!removeEmpty || substrStartIdx <= (int)originalString.size() - 1 && substrStartIdx > 0)
	{
		splitStrings.push_back(std::string(originalString, substrStartIdx));
	}	
	return splitStrings;
}

std::string ToLower(std::string const& string)
{
	std::string lowerString = string;
	for (int c = 0; c < (int)string.size(); c++)
	{
		lowerString[c] = (char)tolower((int)string[c]);
	}
	return lowerString;
}

void TrimString(std::string& originalString, char delimiterToTrim)
{
	std::string trimmed;
	for (int c = 0; c < (int)originalString.size(); c++)
	{
		if (originalString[c] != delimiterToTrim)
		{
			trimmed += originalString[c];
		}
	}
	originalString = trimmed;
}

void TrimString(std::string& originalString, std::string delimiterToTrim)
{
	std::string trimmed;
	for (int c = 0; c < (int)originalString.size(); )
	{
		//we would go over if we tried to look for the entire delimiter to trim
		if (c + delimiterToTrim.size() > originalString.size())
		{
			trimmed += originalString.substr(c);
			break;
		}

		//making i outside so it can be used to advance c once checking for delimiter is done
		int i = 0;
		for (; i < (int)delimiterToTrim.size(); i++)
		{
			if (originalString[c + i] != delimiterToTrim[i])
			{
				trimmed += originalString.substr(c, i + 1);
				break;
			}
		}
		c += (i + 1);
	}
	originalString = trimmed;
}

Strings SplitStringWithQuotes(std::string const& originalString, char delimiterToSplitOn)
{
	Strings splitStrings;
	int substrStartIdx = 0;
	bool foundOpenQuote = false;
	for (int i = 0; i < static_cast<int>(originalString.length()); i++)
	{
		if (originalString[i] == '\"')
		{
			if (!foundOpenQuote)
			{
				foundOpenQuote = true;
			}
			else
			{
				foundOpenQuote = false;
			}
		}
		if (originalString[i] == delimiterToSplitOn && !foundOpenQuote)
		{
			int substringSize = i - substrStartIdx;
			if (substringSize == 0)
			{
				substrStartIdx++;
				continue;
			}
			splitStrings.push_back(std::string(originalString, substrStartIdx, substringSize));
			substrStartIdx = i + 1;
		}
	}
	if (substrStartIdx <= (int)originalString.size() - 1 && substrStartIdx >= 0)
	{
		splitStrings.push_back(std::string(originalString, substrStartIdx));
	}
	return splitStrings;
}

bool FilePathCompare(std::string const& filePath1, std::string const& filePath2)
{
	Strings filePath1Split = SplitStringOnDelimiter(filePath1, '/');
	std::string file1 = filePath1Split.size() > 0 ? filePath1Split[(int)filePath1Split.size() - 1] : filePath1;
	Strings filePath2Split = SplitStringOnDelimiter(filePath2, '/');
	std::string file2 = filePath2Split.size() > 0 ? filePath2Split[(int)filePath2Split.size() - 1] : filePath2;

	return file1 == file2;
}





