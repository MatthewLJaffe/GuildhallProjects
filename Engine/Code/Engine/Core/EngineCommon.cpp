#include "Engine/Core/EngineCommon.hpp"

NamedStrings g_gameConfigBlackboard;

std::string GetShaderFilePath(std::string shaderName)
{
	std::string shaderFilepath = g_gameConfigBlackboard.GetValue("engineDataPath", "not found");
	if (shaderFilepath == "not found")
	{
		ERROR_AND_DIE("Did not specify where to get paticle system shaders");
	}
	shaderFilepath += ("/Shaders/" + shaderName);
	return shaderFilepath;
}
