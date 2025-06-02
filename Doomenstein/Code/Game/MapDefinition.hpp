#pragma once
#include "Game/Definition.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "SpawnInfo.hpp"

class Shader;
class Texture;
class SpriteSheet;

class MapDefinition : Definition
{
public:
	//Methods all definitions should have
	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static MapDefinition const* GetByName(const std::string& name);
	static std::vector<MapDefinition const*> s_mapDefinitions;
	bool LoadFromXmlElement(XmlElement const& element) override;

public:
	std::vector<SpawnInfo> m_spawnInfos;
	Image m_tilesImage;
	Image m_actorsImage;
	Shader* m_shader = nullptr;
	SpriteSheet* m_spriteSheet;
private:
	void AddActorDefsFromImage();
};