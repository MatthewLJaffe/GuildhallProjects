#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

typedef std::vector<std::string> Strings;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );
Strings SplitStringOnDelimiter( std::string const& originalString, char delimiterToSplitOn, bool removeEmpty = false);
Strings SplitStringOnDelimiter(std::string const& originalString, std::string const& delimiterToSplitOn, bool removeEmpty = false);
std::string ToLower(std::string const& string);
void TrimString(std::string& originalString, char delimiterToTrim);
void TrimString(std::string& originalString, std::string delimiterToTrim);
Strings SplitStringWithQuotes(std::string const& originalString, char delimiterToSplitOn);
bool FilePathCompare(std::string const& filePath1, std::string const& filePath2);
