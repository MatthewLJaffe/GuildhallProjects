#include "UnitDefinition.hpp"
#include "Game/GameCommon.hpp"

std::vector<UnitDefinition> UnitDefinition::s_unitDefinitions;

void UnitDefinition::LoadDefinitions(std::string filePath)
{
	s_unitDefinitions.emplace_back();
	XmlDocument unitDefsDocument;
	GUARANTEE_OR_DIE(unitDefsDocument.LoadFile(filePath.c_str()) == 0, Stringf("Failed to load file %s", filePath.c_str()));
	XmlElement* rootElement = unitDefsDocument.FirstChildElement("UnitDefinitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for effect config %s", filePath.c_str()));



	for (XmlElement const* unitElement = rootElement->FirstChildElement("UnitDefinition"); unitElement != nullptr; unitElement = unitElement->NextSiblingElement())
	{
		s_unitDefinitions.emplace_back();
		UnitDefinition& unitDef = s_unitDefinitions[(int)s_unitDefinitions.size() - 1];
		unitDef.m_name = ParseXmlAttribute(*unitElement, "name", "Missing");
		unitDef.m_symbol = ParseXmlAttribute(*unitElement, "symbol", '?');
		std::string imageFilePath = ParseXmlAttribute(*unitElement, "imageFilename", "Missing");
		unitDef.m_image = g_theRenderer->CreateOrGetTextureFromFile(imageFilePath.c_str());
		std::string modelFilePath = ParseXmlAttribute(*unitElement, "modelFilename", "Missing");
		unitDef.m_model = new Model(modelFilePath);
		std::string unitType = ParseXmlAttribute(*unitElement, "type", "None");
		if (unitType == "Tank")
		{
			unitDef.m_unitType = UnitType::TANK;
		}
		else if (unitType == "Artillery")
		{
			unitDef.m_unitType = UnitType::ARTILERY;
		}
		unitDef.m_attackDamage = ParseXmlAttribute(*unitElement, "groundAttackDamage", -1.f);
		unitDef.m_attackRangeMin = ParseXmlAttribute(*unitElement, "groundAttackRangeMin", -1);
		unitDef.m_attackRangeMax = ParseXmlAttribute(*unitElement, "groundAttackRangeMax", -1);
		unitDef.m_movementRange = ParseXmlAttribute(*unitElement, "movementRange", -1);
		unitDef.m_defense = ParseXmlAttribute(*unitElement, "defense", -1.f);
		unitDef.m_health = ParseXmlAttribute(*unitElement, "health", -1);
	}
}

UnitDefinition UnitDefinition::GetDefinition(std::string name)
{
	for (int i = 0; i < (int)s_unitDefinitions.size(); i++)
	{
		if (s_unitDefinitions[i].m_name == name)
		{
			return s_unitDefinitions[i];
		}
	}
	return s_unitDefinitions[0];
}

UnitDefinition UnitDefinition::GetDefinition(char symbol)
{
	for (int i = 0; i < (int)s_unitDefinitions.size(); i++)
	{
		if (s_unitDefinitions[i].m_symbol == symbol)
		{
			return s_unitDefinitions[i];
		}
	}
	return s_unitDefinitions[0];
}
