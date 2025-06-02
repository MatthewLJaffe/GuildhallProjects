#include "Engine/Renderer/MtlLib.hpp"
#include "Engine/Core/FileUtils.hpp"

MtlLib::MtlLib(const char* filePath)
	: m_filePath(filePath)
{
	std::string mtlLib;
	bool valid = FileReadToString(mtlLib, m_filePath);
	if (!valid)
	{
		ERROR_RECOVERABLE("Could not find mtl file");
		return;
	}
	Strings mtlLines = SplitStringOnDelimiter(mtlLib, "\r\n", true);
	if ((int)mtlLines.size() <= 1)
	{
		mtlLines = SplitStringOnDelimiter(mtlLib, "\n", true);
	}
	std::string currMtlKey;
	for (int i = 0; i < (int)mtlLines.size(); i++)
	{
		if (mtlLines[i].substr(0, 7) == "newmtl ")
		{
			currMtlKey = SplitStringOnDelimiter(mtlLines[i], ' ', true)[1];
			m_mtls[currMtlKey] = Mtl(currMtlKey);
		}
		if (mtlLines[i].substr(0, 3) == "Kd ")
		{
			Strings diffuseValuesSplit = SplitStringOnDelimiter(mtlLines[i], ' ', true);
			float colorAsFloats[4];
			colorAsFloats[0] = (float)atof(diffuseValuesSplit[1].c_str());
			colorAsFloats[1] = (float)atof(diffuseValuesSplit[2].c_str());
			colorAsFloats[2] = (float)atof(diffuseValuesSplit[3].c_str());
			if (diffuseValuesSplit.size() == 5)
			{
				colorAsFloats[3] = (float)atof(diffuseValuesSplit[4].c_str());
			}
			else
			{
				colorAsFloats[3] = 1.0f;
			}
			m_mtls[currMtlKey].m_diffuse.SetFromFloats(colorAsFloats);
		}
	}
}

Mtl::Mtl(std::string name)
	: m_name(name)
{
}
