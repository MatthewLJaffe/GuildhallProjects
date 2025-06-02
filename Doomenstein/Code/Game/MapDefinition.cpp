#include "MapDefinition.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/ActorDefinition.hpp"

class Shader;
class Texture;

std::vector<MapDefinition const*> MapDefinition::s_mapDefinitions;


void MapDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile(path) == 0, std::string("Failed to load ") + std::string("Data/Definitions/MapDefinitions.xml"));
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("Definitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the root GameConfig element from ") + std::string("Data/Definitions/MapDefinitions.xml"));
	for (XmlElement const* currElement = rootElement->FirstChildElement(); currElement != nullptr; currElement = currElement->NextSiblingElement())
	{
		MapDefinition* mapDef = new MapDefinition();
		mapDef->LoadFromXmlElement(*currElement);
		s_mapDefinitions.push_back(mapDef);
	}
}

void MapDefinition::ClearDefinitions()
{
	for (size_t i = 0; i < s_mapDefinitions.size(); i++)
	{
		delete s_mapDefinitions[i];
		s_mapDefinitions[i] = nullptr;
	}
	s_mapDefinitions.clear();
}

MapDefinition const* MapDefinition::GetByName(const std::string& name)
{
	for (size_t i = 0; i < s_mapDefinitions.size(); i++)
	{
		if (s_mapDefinitions[i]->m_name == name)
		{
			return s_mapDefinitions[i];
		}
	}
	GUARANTEE_OR_DIE(false, "Failed to get map definition from name " + name);
	return nullptr;
}

bool MapDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", "missing name");
	m_tilesImage = Image(ParseXmlAttribute(element, "image", "path not found").c_str());
	std::string actorsImageFilePath = ParseXmlAttribute(element, "actorsImage", "path not found");
	if (actorsImageFilePath != "path not found")
	{
		m_actorsImage = Image(actorsImageFilePath.c_str());
		AddActorDefsFromImage();
	}
	m_shader = g_theRenderer->CreateOrGetShaderFromFile(ParseXmlAttribute(element, "shader", "not found").c_str(), VertexType::VERTEX_TYPE_PCUTBN);
	IntVec2 spriteSheetDimensions = ParseXmlAttribute(element, "spriteSheetCellCount", IntVec2(-1, -1));
	Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(element, "spriteSheetTexture", "texture not found").c_str());
	m_spriteSheet = new SpriteSheet(spriteTexture, spriteSheetDimensions);

	XmlElement const* spawnInfosElement = element.FirstChildElement("SpawnInfos");
	if (spawnInfosElement != nullptr)
	{
		for (XmlElement const* currElement = spawnInfosElement->FirstChildElement(); currElement != nullptr; currElement = currElement->NextSiblingElement())
		{
			SpawnInfo currSpawnInfo;
			std::string actorName = ParseXmlAttribute(*currElement, "actor", "No actor found");
			currSpawnInfo.m_actorDefinition = ActorDefinition::GetByName(actorName);
			currSpawnInfo.m_orientation = ParseXmlAttribute(*currElement, "orientation", EulerAngles::IDENTITY);
			currSpawnInfo.m_position = ParseXmlAttribute(*currElement, "position", Vec3::ZERO);
			currSpawnInfo.m_velocity = ParseXmlAttribute(*currElement, "velocity", Vec3::ZERO);
			m_spawnInfos.push_back(currSpawnInfo);
		}
	}

	return true;
}

void MapDefinition::AddActorDefsFromImage()
{
	IntVec2 dimensions = m_actorsImage.GetDimensions();
	for (int y = 0; y < dimensions.y; y++)
	{
		for (int x = 0; x < dimensions.x; x++)
		{
			//spawn actors
			Rgba8 actorColor = m_actorsImage.GetTexelColor(IntVec2(x, y));
			ActorDefinition const* actorDef = ActorDefinition::GetByColor(actorColor);
			if (actorDef != nullptr)
			{
				SpawnInfo actorSpawnInfo;
				actorSpawnInfo.m_actorDefinition = actorDef;
				actorSpawnInfo.m_orientation.m_yaw = RangeMap((float)actorColor.a, 0.f, 255.f, 0.f, 360.f);
				actorSpawnInfo.m_position = Vec3((float)x + .5f, (float)y + .5f, 0.f);
				m_spawnInfos.push_back(actorSpawnInfo);
			}
		}
	}
}
