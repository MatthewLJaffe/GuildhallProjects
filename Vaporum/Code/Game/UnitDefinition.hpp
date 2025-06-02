#pragma once
#include "Game/Model.hpp"

enum class UnitType
{
	NONE,
	TANK,
	ARTILERY
};

struct UnitDefinition
{
	static std::vector<UnitDefinition> s_unitDefinitions;
	static void LoadDefinitions(std::string filePath);
	static UnitDefinition GetDefinition(std::string name);
	static UnitDefinition GetDefinition(char symbol);

	char m_symbol = '?';
	std::string m_name = "Missing";
	Texture* m_image = nullptr;
	Model* m_model = nullptr;
	UnitType m_unitType = UnitType::NONE;
	float m_attackDamage = -1.f;
	int m_attackRangeMin = -1;
	int m_attackRangeMax = -1;
	int m_movementRange = -1;
	float m_defense = -1.f;
	int m_health = -1;
};