#pragma once
#include "Game/GameCommon.hpp"

class Actor;

struct SteeringBehaviourInitializer
{
	float m_weight = 0.f;
	std::string m_type = "";
	bool m_strafe = false;
};

class SteeringBehaviour
{
public:
	SteeringBehaviour(SteeringBehaviourInitializer const& initializer, Actor const& actor);
	virtual ~SteeringBehaviour() = default;
	static SteeringBehaviour* ConstructFromInitializer(SteeringBehaviourInitializer const& initializer, Actor const& owningActor);
	virtual Vec3 GetWeightedSteerDireciton() const = 0;
	float m_weight = 0.f;
	bool m_strafe = false;
	Actor const& m_actor;
};

class ChasePlayerSB : public SteeringBehaviour
{
	friend class SteeringBehaviour;
public:
	virtual Vec3 GetWeightedSteerDireciton() const override;
protected:
	ChasePlayerSB(SteeringBehaviourInitializer const& initializer, Actor const& m_actor);
};

class AvoidObstaclesSB : public SteeringBehaviour
{
	friend class SteeringBehaviour;
public:
	virtual Vec3 GetWeightedSteerDireciton() const override;
protected:
	AvoidObstaclesSB(SteeringBehaviourInitializer const& initializer, Actor const& m_actor);
};

class CirclePlayerSB : public SteeringBehaviour
{
	friend class SteeringBehaviour;
public:
	virtual Vec3 GetWeightedSteerDireciton() const override;
protected:
	CirclePlayerSB(SteeringBehaviourInitializer const& initializer, Actor const& m_actor);
};

class LeftSB : public SteeringBehaviour
{
	friend class SteeringBehaviour;
public:
	virtual Vec3 GetWeightedSteerDireciton() const override;
protected:
	LeftSB(SteeringBehaviourInitializer const& initializer, Actor const& m_actor);
};