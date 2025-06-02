#pragma once
#include <stdio.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

enum class BufferEndianMode
{
	LITTLE = 1,
	BIG = 2
};

bool FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename);
bool FileReadToBuffer(std::vector<uint8_t>& outBuffer, FILE* filePtr);
bool FileReadToString(std::string& outString, const std::string& filename);
bool WriteBufferToFile(std::vector<uint8_t> const& inBuffer, std::string const& fileName);
bool WriteStringToFile(std::string const& inString, std::string const& fileName);
BufferEndianMode GetNativeSystemEndian();



class BufferWriter
{
public:
	BufferWriter(std::vector<uint8_t>& byteBuffer);
	void SetEndianMode(BufferEndianMode endianMode);
	void AppendChar(char c);
	void AppendByte(uint8_t byte);
	void AppendBool(bool b);
	void AppendTwoBytes(uint8_t* ptrToTwoBytes);
	void AppendFourBytes(uint8_t* ptrToFourBytes);
	void AppendEightBytes(uint8_t* ptrToEightBytes);

	void AppendUint32(unsigned int uint);
	void AppendUshort(unsigned short ushort);
	void AppendShort(short s);
	void AppendUint64(uint64_t bigUint);
	void AppendInt64(int64_t bigInt);
	void WriteUint32AtOffset(unsigned int offsetInBuff, unsigned int uint32);

	void AppendInt32(int i);
	void AppendFloat(float f);
	void AppendDouble(double d);
	void AppendStringZeroTerminated(std::string const& string);
	void AppendStringAfter32BitLength(std::string const& string);
	void AppendRgba(Rgba8 const& rgba);
	void AppendRgb(Rgba8 const& rgba);
	void AppendIntVec2(IntVec2 const& intVec2);
	void AppendVec2(Vec2 const& vec2);
	void AppendVec3(Vec3 const& vec3);
	void AppendVertexPCU(Vertex_PCU const& vertexPCU);
	BufferEndianMode GetEndianMode();
	size_t GetTotalSize();
	size_t GetAppendedSize();
	void AppendByteBuffer(std::vector<uint8_t> const& bytesToApend);
	std::vector<uint8_t>& m_byteBuffer;
private:
	BufferEndianMode m_endianMode = BufferEndianMode::LITTLE;
	size_t m_appendedSize = 0;
};

class BufferParser
{
public:
	BufferParser(std::vector<uint8_t>& byteBuffer);
	void SetEndianMode(BufferEndianMode endianMode);
	char ParseChar();
	unsigned char ParseByte();
	bool ParseBool();
	void Parse2Bytes(uint8_t* ptrToTwoBytes);
	void Parse4Bytes(uint8_t* ptrToFourBytes);
	void Parse8Bytes(uint8_t* ptrToFourBytes);
	unsigned short ParseUshort();
	short ParseShort();

	unsigned int ParseUint32();
	int ParseInt32();
	uint64_t ParseUint64();
	int64_t ParseInt64();
	float ParseFloat();
	double ParseDouble();
	void ParseStringZeroTerminated(std::string& outputString);
	void ParseStringAfter32BitLength(std::string& outputString);
	Rgba8 ParseRgba8();
	Rgba8 ParseRgb();
	IntVec2 ParseIntVec2();
	Vec2 ParseVec2();
	Vertex_PCU ParseVertex_PCU();
	size_t GetRemainingSize();
	void SetCurrentBytePosition(int currentBytePosition);
	int GetCurrentBytePosition();




private:
	BufferEndianMode m_endianMode = BufferEndianMode::LITTLE;
	std::vector<uint8_t> m_byteBuffer;
	int m_currentBytePosition = 0;
};