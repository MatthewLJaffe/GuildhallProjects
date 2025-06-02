#pragma once
#include "Game/UnitDefinition.hpp"
#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"

class Unit : public Entity
{
public:
	Unit(Game* game, IntVec2 const& coords, UnitDefinition const& definition, bool player1);
	void Update(float deltaSeconds) override;
	void Render() const override;
	bool CanMoveToTile(IntVec2 const& tile) const;
	bool CanAttackTile(IntVec2 const& tile) const;
	void Die();
	void Move();
	void Attack();
	int m_currentHealth = 0;
	UnitDefinition m_def;
	Team m_team;
	IntVec2 m_coords;
	IntVec2 m_previousCoords;
	IntVec2 m_goalCoords;
	std::vector<int> m_heatMap;
	std::vector<Vertex_PCU> m_gridVerts;
	bool m_actionsFinished = false;
	bool m_isStaying = false;
	void UpdateHeatMap();

	bool m_isAttacking = false;

	//for movement
	bool m_isMoving = false;
	bool m_moveStarted = false;
	float m_rotationTowardsFocusedTileSpeed = 180.f;
	float m_previousTileRotation = 0.f;

	CatmullRomSpline m_currentMoveCurve;
	float m_currentMoveT = 0.f;
	float m_maxMoveT = 0.f;
	float m_tileMoveSpeed = 5.f;
	Vec2 m_firstTilePos;

	Vec3 m_previousFramePosition;
private:
	void AddVertsForReachableTiles(std::vector<Vertex_PCU>& verts) const;
	void AddVertsForMovePath(std::vector<Vertex_PCU>& verts) const;
	void AddVertsForAttackableTiles(std::vector<Vertex_PCU>& verts) const;
	void GetMovePath(std::vector<IntVec2>& out_path) const;
	void UpdateMove(float deltaSeconds);
};