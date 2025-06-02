#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <map>

struct Mtl
{
	Mtl() = default;
	Mtl(std::string name);
	std::string m_name;
	Rgba8 m_diffuse;
};

class MtlLib
{
	friend class Renderer;
public:
	std::string m_filePath;
	std::map<std::string, Mtl> m_mtls;

private:
	MtlLib(const char* filePath);
};