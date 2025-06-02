#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <iostream>

bool FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	FILE* filePtr = nullptr;
	errno_t error = fopen_s(&filePtr, filename.c_str(), "rb");
	if (error != 0)
	{
		ERROR_AND_DIE("Failed to open file " + filename);
	}
	fseek(filePtr, 0, SEEK_END);
	size_t fileSize = (size_t)ftell(filePtr);
	outBuffer.resize(fileSize);
	fseek(filePtr, 0, SEEK_SET);
	size_t numBytesRead = fread(outBuffer.data(), 1, fileSize, filePtr);
	bool correctNumBytesRead = numBytesRead == fileSize;
	fclose(filePtr);
	return correctNumBytesRead;
}

bool FileReadToBuffer(std::vector<uint8_t>& outBuffer, FILE* filePtr)
{
	if (outBuffer.size() > 0)
	{
		outBuffer.clear();
	}
	fseek(filePtr, 0, SEEK_END);
	size_t fileSize = (size_t)ftell(filePtr);
	outBuffer.resize(fileSize);
	fseek(filePtr, 0, SEEK_SET);
	size_t numBytesRead = fread(outBuffer.data(), 1, fileSize, filePtr);
	bool correctNumBytesRead = numBytesRead == fileSize;
	fclose(filePtr);
	return correctNumBytesRead;
}

bool FileReadToString(std::string& outString, const std::string& filename)
{
	std::vector<uint8_t> byteBuffer;
	bool correctNumBytesRead = FileReadToBuffer(byteBuffer, filename);
	if (!correctNumBytesRead)
	{
		ERROR_RECOVERABLE("incorrect number of bytes read for file " + filename);
		return false;
	}
	byteBuffer.push_back('\0');
	outString = std::string(reinterpret_cast<char*>(byteBuffer.data()), byteBuffer.size());
	return correctNumBytesRead;
}

bool WriteBufferToFile(std::vector<uint8_t> const& inBuffer, std::string const& fileName)
{
	FILE* filePtr = nullptr;
	
	errno_t error = fopen_s(&filePtr, fileName.c_str(), "wb");
	if (error != 0)
	{
		ERROR_AND_DIE("Failed to open file " + fileName);
	}
	fwrite(inBuffer.data(), sizeof(uint8_t), inBuffer.size(), filePtr);
	fseek(filePtr, 0, SEEK_END);
	size_t fileSize = (size_t)ftell(filePtr);
	bool correctNumBytesWritten = inBuffer.size() == fileSize;
	fclose(filePtr);
	return correctNumBytesWritten;
}

bool WriteStringToFile(std::string const& inString, std::string const& fileName)
{
	FILE* filePtr = nullptr;
	errno_t error = fopen_s(&filePtr, fileName.c_str(), "w");
	if (error != 0)
	{
		ERROR_AND_DIE("Failed to open file " + fileName);
	}
	fwrite(inString.data(), sizeof(uint8_t), inString.size(), filePtr);
	fseek(filePtr, 0, SEEK_END);
	size_t fileSize = (size_t)ftell(filePtr);
	bool correctNumBytesWritten = inString.size() == fileSize;
	fclose(filePtr);
	return correctNumBytesWritten;
}

BufferEndianMode GetNativeSystemEndian()
{
	unsigned short twoBytes = 0x1234;
	uint8_t* twoBytesAsIntsPtr = (uint8_t*)&twoBytes;
	if (twoBytesAsIntsPtr[0] == 0x12)
	{
		return BufferEndianMode::BIG;
	}
	else if (twoBytesAsIntsPtr[0] == 0x34)
	{
		return BufferEndianMode::LITTLE;
	}
	else
	{
		ERROR_AND_DIE("WHUT?");
	}
}

BufferParser::BufferParser(std::vector<uint8_t>& byteBuffer)
	: m_byteBuffer(byteBuffer)
{
	m_endianMode = GetNativeSystemEndian();

}


void BufferParser::SetEndianMode(BufferEndianMode endianMode)
{
	m_endianMode = endianMode;
}

char BufferParser::ParseChar()
{
	if (m_currentBytePosition >= (int)m_byteBuffer.size())
	{
		ERROR_AND_DIE("Trying to parse outside of buffer");
	}
	char parsedChar = (char)m_byteBuffer[m_currentBytePosition];
	m_currentBytePosition++;
	return parsedChar;
}

unsigned char BufferParser::ParseByte()
{
	if (m_currentBytePosition >= (int)m_byteBuffer.size())
	{
		ERROR_AND_DIE("Trying to parse outside of buffer");
	}
	unsigned char parsedByte = (unsigned char)m_byteBuffer[m_currentBytePosition];
	m_currentBytePosition++;
	return parsedByte;
}

bool BufferParser::ParseBool()
{
	if (m_currentBytePosition >= (int)m_byteBuffer.size())
	{
		ERROR_AND_DIE("Trying to parse outside of buffer");
	}
	bool parsedBool = (bool)m_byteBuffer[m_currentBytePosition];
	m_currentBytePosition++;
	return parsedBool;
}

void BufferParser::Parse2Bytes(uint8_t* ptrToTwoBytes)
{
	uint8_t bytes[2];

	bytes[0] = ParseByte();
	bytes[1] = ParseByte();

	if (m_endianMode == GetNativeSystemEndian())
	{
		ptrToTwoBytes[0] = bytes[0];
		ptrToTwoBytes[1] = bytes[1];
	}
	else
	{
		ptrToTwoBytes[0] = bytes[1];
		ptrToTwoBytes[1] = bytes[0];
	}
}

void BufferParser::Parse4Bytes(uint8_t* ptrToFourBytes)
{
	uint8_t bytes[4];

	bytes[0] = ParseByte();
	bytes[1] = ParseByte();
	bytes[2] = ParseByte();
	bytes[3] = ParseByte();

	if (m_endianMode == GetNativeSystemEndian())
	{
		ptrToFourBytes[0] = bytes[0];
		ptrToFourBytes[1] = bytes[1];
		ptrToFourBytes[2] = bytes[2];
		ptrToFourBytes[3] = bytes[3];

	}
	else
	{
		ptrToFourBytes[0] = bytes[3];
		ptrToFourBytes[1] = bytes[2];
		ptrToFourBytes[2] = bytes[1];
		ptrToFourBytes[3] = bytes[0];
	}
}

void BufferParser::Parse8Bytes(uint8_t* ptrToFourBytes)
{
	uint8_t bytes[8];

	bytes[0] = ParseByte();
	bytes[1] = ParseByte();
	bytes[2] = ParseByte();
	bytes[3] = ParseByte();
	bytes[4] = ParseByte();
	bytes[5] = ParseByte();
	bytes[6] = ParseByte();
	bytes[7] = ParseByte();


	if (m_endianMode == GetNativeSystemEndian())
	{
		ptrToFourBytes[0] = bytes[0];
		ptrToFourBytes[1] = bytes[1];
		ptrToFourBytes[2] = bytes[2];
		ptrToFourBytes[3] = bytes[3];
		ptrToFourBytes[4] = bytes[4];
		ptrToFourBytes[5] = bytes[5];
		ptrToFourBytes[6] = bytes[6];
		ptrToFourBytes[7] = bytes[7];
	}
	else
	{
		ptrToFourBytes[0] = bytes[7];
		ptrToFourBytes[1] = bytes[6];
		ptrToFourBytes[2] = bytes[5];
		ptrToFourBytes[3] = bytes[4];
		ptrToFourBytes[4] = bytes[3];
		ptrToFourBytes[5] = bytes[2];
		ptrToFourBytes[6] = bytes[1];
		ptrToFourBytes[7] = bytes[0];
	}
}

unsigned short BufferParser::ParseUshort()
{
	uint8_t bytes[2];
	Parse2Bytes(bytes);
	unsigned short* bytesAsUShort = reinterpret_cast<unsigned short*>(bytes);
	return *bytesAsUShort;
}

short BufferParser::ParseShort()
{
	uint8_t bytes[2];
	Parse2Bytes(bytes);
	short* bytesAsShort = reinterpret_cast<short*>(bytes);
	return *bytesAsShort;
}


unsigned int BufferParser::ParseUint32()
{
	uint8_t bytes[4];
	Parse4Bytes(bytes);
	unsigned int* bytesAsUInt = reinterpret_cast<unsigned int*>(bytes);
	return *bytesAsUInt;
}

int BufferParser::ParseInt32()
{
	uint8_t bytes[4];
	Parse4Bytes(bytes);
	int* bytesAsInt = reinterpret_cast<int*>(bytes);
	return *bytesAsInt;
}

uint64_t BufferParser::ParseUint64()
{
	uint8_t bytes[8];
	Parse8Bytes(bytes);
	uint64_t* bytesAsUint64 = reinterpret_cast<uint64_t*>(bytes);
	return *bytesAsUint64;
}

int64_t BufferParser::ParseInt64()
{
	uint8_t bytes[8];
	Parse8Bytes(bytes);
	int64_t* bytesAsInt64 = reinterpret_cast<int64_t*>(bytes);
	return *bytesAsInt64;
}

float BufferParser::ParseFloat()
{
	uint8_t bytes[4];
	Parse4Bytes(bytes);
	float* bytesAsFloat = reinterpret_cast<float*>(bytes);
	return *bytesAsFloat;
}

double BufferParser::ParseDouble()
{
	uint8_t bytes[8];
	Parse8Bytes(bytes);
	double* bytesAsDouble = reinterpret_cast<double*>(bytes);
	return *bytesAsDouble;
}

void BufferParser::ParseStringZeroTerminated(std::string& outputString)
{
	outputString = "";
	for (char c = ParseChar(); c != '\0'; c = ParseChar())
	{
		outputString += c;
	}
}

void BufferParser::ParseStringAfter32BitLength(std::string& outputString)
{
	outputString = "";
	unsigned int length = ParseUint32();
	for (unsigned int i = 0; i < length; i++)
	{
		outputString += ParseChar();
	}
}

Rgba8 BufferParser::ParseRgba8()
{
	Rgba8 parsedColor;
	parsedColor.r = ParseByte();
	parsedColor.g = ParseByte();
	parsedColor.b = ParseByte();
	parsedColor.a = ParseByte();

	return parsedColor;
}

Rgba8 BufferParser::ParseRgb()
{
	Rgba8 parsedColor;
	parsedColor.r = ParseByte();
	parsedColor.g = ParseByte();
	parsedColor.b = ParseByte();
	parsedColor.a = 255;

	return parsedColor;
}

IntVec2 BufferParser::ParseIntVec2()
{
	IntVec2 parsedIntVec2;
	parsedIntVec2.x = ParseInt32();
	parsedIntVec2.y = ParseInt32();
	return parsedIntVec2;
}

Vec2 BufferParser::ParseVec2()
{
	Vec2 parsedVec2;
	parsedVec2.x = ParseFloat();
	parsedVec2.y = ParseFloat();
	return parsedVec2;
}

Vertex_PCU BufferParser::ParseVertex_PCU()
{
	Vertex_PCU vertex;
	vertex.m_position.x = ParseFloat();
	vertex.m_position.y = ParseFloat();
	vertex.m_position.z = ParseFloat();

	vertex.m_color = ParseRgba8();

	vertex.m_uvTexCoords = ParseVec2();

	return vertex;
}

size_t BufferParser::GetRemainingSize()
{
	return m_byteBuffer.size() - m_currentBytePosition;
}

void BufferParser::SetCurrentBytePosition(int currentBytePosition)
{
	m_currentBytePosition = currentBytePosition;
}

int BufferParser::GetCurrentBytePosition()
{
	return m_currentBytePosition;
}


BufferWriter::BufferWriter(std::vector<uint8_t>& byteBuffer)
	: m_byteBuffer(byteBuffer)
{
	m_endianMode = GetNativeSystemEndian();
}

void BufferWriter::SetEndianMode(BufferEndianMode endianMode)
{
	m_endianMode = endianMode;
}

void BufferWriter::AppendChar(char c)
{
	m_byteBuffer.push_back((uint8_t)c);
	m_appendedSize++;

}

void BufferWriter::AppendByte(uint8_t byte)
{
	m_byteBuffer.push_back(byte);
	m_appendedSize++;

}

void BufferWriter::AppendBool(bool b)
{
	m_byteBuffer.push_back((bool)b);
	m_appendedSize ++;
}

void BufferWriter::AppendTwoBytes(uint8_t* ptrToTwoBytes)
{
	if (m_endianMode == GetNativeSystemEndian())
	{
		m_byteBuffer.push_back(ptrToTwoBytes[0]);
		m_byteBuffer.push_back(ptrToTwoBytes[1]);
	}
	else
	{
		m_byteBuffer.push_back(ptrToTwoBytes[1]);
		m_byteBuffer.push_back(ptrToTwoBytes[0]);
	}
}

void BufferWriter::AppendFourBytes(uint8_t* ptrToFourBytes)
{
	if (m_endianMode == GetNativeSystemEndian())
	{
		m_byteBuffer.push_back(ptrToFourBytes[0]);
		m_byteBuffer.push_back(ptrToFourBytes[1]);
		m_byteBuffer.push_back(ptrToFourBytes[2]);
		m_byteBuffer.push_back(ptrToFourBytes[3]);

	}
	else
	{
		m_byteBuffer.push_back(ptrToFourBytes[3]);
		m_byteBuffer.push_back(ptrToFourBytes[2]);
		m_byteBuffer.push_back(ptrToFourBytes[1]);
		m_byteBuffer.push_back(ptrToFourBytes[0]);
	}
}

void BufferWriter::AppendEightBytes(uint8_t* ptrToEightBytes)
{
	if (m_endianMode == GetNativeSystemEndian())
	{
		m_byteBuffer.push_back(ptrToEightBytes[0]);
		m_byteBuffer.push_back(ptrToEightBytes[1]);
		m_byteBuffer.push_back(ptrToEightBytes[2]);
		m_byteBuffer.push_back(ptrToEightBytes[3]);
		m_byteBuffer.push_back(ptrToEightBytes[4]);
		m_byteBuffer.push_back(ptrToEightBytes[5]);
		m_byteBuffer.push_back(ptrToEightBytes[6]);
		m_byteBuffer.push_back(ptrToEightBytes[7]);

	}
	else
	{
		m_byteBuffer.push_back(ptrToEightBytes[7]);
		m_byteBuffer.push_back(ptrToEightBytes[6]);
		m_byteBuffer.push_back(ptrToEightBytes[5]);
		m_byteBuffer.push_back(ptrToEightBytes[4]);
		m_byteBuffer.push_back(ptrToEightBytes[3]);
		m_byteBuffer.push_back(ptrToEightBytes[2]);
		m_byteBuffer.push_back(ptrToEightBytes[1]);
		m_byteBuffer.push_back(ptrToEightBytes[0]);
	}
}

void BufferWriter::AppendUint32(unsigned int uint)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&uint);
	AppendFourBytes(bytes);
	m_appendedSize += 4;

}
void BufferWriter::WriteUint32AtOffset(unsigned int offsetInBuff, unsigned int uint32)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&uint32);
	if (m_endianMode == GetNativeSystemEndian())
	{
		m_byteBuffer[offsetInBuff + 0] = bytes[0];
		m_byteBuffer[offsetInBuff + 1] = bytes[1];
		m_byteBuffer[offsetInBuff + 2] = bytes[2];
		m_byteBuffer[offsetInBuff + 3] = bytes[3];
	}
	else
	{
		m_byteBuffer[offsetInBuff + 0] = bytes[3];
		m_byteBuffer[offsetInBuff + 1] = bytes[2];
		m_byteBuffer[offsetInBuff + 2] = bytes[1];
		m_byteBuffer[offsetInBuff + 3] = bytes[0];
	}
}


void BufferWriter::AppendUshort(unsigned short ushort)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&ushort);
	AppendTwoBytes(bytes);
	m_appendedSize += 2;
}

void BufferWriter::AppendShort(short s)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&s);
	AppendTwoBytes(bytes);
	m_appendedSize += 2;
}

void BufferWriter::AppendUint64(uint64_t bigUint)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&bigUint);
	AppendEightBytes(bytes);
	m_appendedSize += 8;
}

void BufferWriter::AppendInt64(int64_t bigInt)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&bigInt);
	AppendEightBytes(bytes);
	m_appendedSize += 8;
}

void BufferWriter::AppendInt32(int i)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&i);
	AppendFourBytes(bytes);
	m_appendedSize += 4;

}

void BufferWriter::AppendFloat(float f)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&f);
	AppendFourBytes(bytes);
	m_appendedSize += 4;

}

void BufferWriter::AppendDouble(double d)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&d);
	AppendEightBytes(bytes);
	m_appendedSize += 8;
}

void BufferWriter::AppendStringZeroTerminated(std::string const& string)
{
	for (int i = 0; i < string.size(); i++)
	{
		AppendChar(string[i]);
	}
	AppendChar('\0');
}

void BufferWriter::AppendStringAfter32BitLength(std::string const& string)
{
	AppendInt32((unsigned int)string.size());
	for (int i = 0; i < string.size(); i++)
	{
		AppendChar(string[i]);
	}
}

void BufferWriter::AppendRgba(Rgba8 const& color)
{
	AppendByte(color.r);
	AppendByte(color.g);
	AppendByte(color.b);
	AppendByte(color.a);
}

void BufferWriter::AppendRgb(Rgba8 const& color)
{
	AppendByte(color.r);
	AppendByte(color.g);
	AppendByte(color.b);
}

void BufferWriter::AppendIntVec2(IntVec2 const& intVec2)
{
	AppendInt32(intVec2.x);
	AppendInt32(intVec2.y);
}

void BufferWriter::AppendVec2(Vec2 const& vec2)
{
	AppendFloat(vec2.x);
	AppendFloat(vec2.y);
}

void BufferWriter::AppendVec3(Vec3 const& vec3)
{
	AppendFloat(vec3.x);
	AppendFloat(vec3.y);
	AppendFloat(vec3.z);
}

void BufferWriter::AppendByteBuffer(std::vector<uint8_t> const& bytesToApend)
{
	size_t sizeBeforeAppend = m_byteBuffer.size();
	m_byteBuffer.resize(m_byteBuffer.size() + bytesToApend.size());
	memcpy((void*)(& m_byteBuffer.data()[sizeBeforeAppend]), bytesToApend.data(), bytesToApend.size());
}


void BufferWriter::AppendVertexPCU(Vertex_PCU const& vertexPCU)
{
	AppendVec3(vertexPCU.m_position);
	AppendRgba(vertexPCU.m_color);
	AppendVec2(vertexPCU.m_uvTexCoords);
}

BufferEndianMode BufferWriter::GetEndianMode()
{
	return m_endianMode;
}

size_t BufferWriter::GetTotalSize()
{
	return m_byteBuffer.size();
}

size_t BufferWriter::GetAppendedSize()
{
	return m_appendedSize;
}
