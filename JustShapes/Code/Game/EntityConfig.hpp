#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"

struct EntityConfig
{
	EntityConfig() = default;
	static EntityConfig GetEntityConfigByName(std::string configName);
	static void InitializeEntityConfigs();
	void LoadFromXmlElement(XmlElement const& entityConfigElement);

	static std::vector<EntityConfig> s_entityConfigs;

	std::string m_name = "invalid";
	bool m_useConfig = false;

	//visual
	float m_startOrientaiton = 0.f;
	Texture* m_texture = nullptr;
	AABB2 m_spriteBounds = AABB2(Vec2::ZERO, Vec2::ONE * 16.f);
	AABB2 m_uvs = AABB2::ZERO_TO_ONE;
	Vec2 m_normalizedPivot = Vec2(.5f, .5f);
	Rgba8 m_startColor = Rgba8::WHITE;
	int m_sortOrder = 0;

	//lifetime
	float m_liveTime = -1.f;
	std::string m_startAnimation = "";
	float m_hideTime = -1.f;

	//float m_maxConeAngle = 30.f;

	//physics
	bool m_simulatePhysics = false;
	bool m_startHazard = false;
	Vec2 m_startVelocity = Vec2::ZERO;
	Vec2 m_startAcceleration = Vec2::ZERO;
	float m_rotationSpeed = 0.f;
	float m_becomeHazardTime = -1.f;
	float m_collisionRadius = 8.f;
};

