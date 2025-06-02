#include "Game/Unit.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

Unit::Unit(Game* game, IntVec2 const& coords, UnitDefinition const& definition, bool isPlayer1)
	: Entity(game, GetHexPosFromGridCoord(coords).GetXYZ())
	, m_coords(coords)
	, m_previousCoords(coords)
	, m_def(definition)
{
	if (isPlayer1)
	{
		m_team = Team::BLUE;
	}
	else
	{
		m_team = Team::RED;
		m_orientationDegrees.m_yaw = 180.f;
		m_previousTileRotation = 180.f;
	}
	m_currentHealth = m_def.m_health;
	m_heatMap.resize(game->m_currentMapDef.m_gridSize.x * game->m_currentMapDef.m_gridSize.y);
	UpdateHeatMap();
}

void Unit::Update(float deltaSeconds)
{
	if ( g_theGame->m_selectedUnit == this && !m_moveStarted)
	{
		Vec2 goalDisp = GetHexPosFromGridCoord(g_theGame->m_focusedHexCoords) - GetHexPosFromGridCoord(m_coords);
		if (m_isMoving)
		{
			goalDisp = m_firstTilePos - m_position.GetXY();
			goalDisp = goalDisp.GetNormalized();
		}
		float currentRotation = m_orientationDegrees.m_yaw;
		float goalRotation = goalDisp.GetOrientationDegrees();
		m_orientationDegrees.m_yaw = GetTurnedTowardDegrees(currentRotation, goalRotation, m_rotationTowardsFocusedTileSpeed * deltaSeconds);
		if (m_isMoving && fabs(m_orientationDegrees.m_yaw - goalRotation) < .1f)
		{
			m_moveStarted = true;
		}
		if (m_isAttacking && fabs(m_orientationDegrees.m_yaw - goalRotation) < .1f)
		{
			m_isAttacking = false;
			g_theGame->AttackComplete();
		}
	}
	else if (m_isMoving && m_moveStarted)
	{
		UpdateMove(deltaSeconds);
	}
}

void Unit::Render() const
{
	Rgba8 color = Rgba8(22, 22, 80);
	if (m_team == Team::RED)
	{
		color = Rgba8(80, 22, 22);
	}
	if (g_theGame->m_selectedUnit == this)
	{
		color.r += 70;
		color.g += 70;
		color.b += 70;
	}
	if (m_actionsFinished)
	{
		if (m_team == Team::RED)
		{
			color = Rgba8(40, 10, 10);
		}
		else
		{
			color = Rgba8(10, 10, 40);
		}
	}
	std::vector<Vertex_PCU> gridVerts;
	if (g_theGame->m_selectedUnit == this)
	{
		g_theGame->AddVertsForHexCell(gridVerts, m_coords, Rgba8(0, 0, 255, 255), true, .8f);
	}
	if (g_theGame->m_selectedUnit == this && m_game->m_currentTurnState == TurnState::MOVING)
	{
		AddVertsForReachableTiles(gridVerts);
		AddVertsForMovePath(gridVerts);
	}
	if (g_theGame->m_selectedUnit == this && m_game->m_currentTurnState == TurnState::ATTACKING)
	{
		AddVertsForAttackableTiles(gridVerts);
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(gridVerts.size(), gridVerts.data());
	g_theRenderer->SetModelConstants(GetModelMatrix(), color);
	m_def.m_model->Render();
}

bool Unit::CanMoveToTile(IntVec2 const& tile) const
{
	for (int i = 0; i < (int)m_game->m_allUnits.size(); i++)
	{
		if (tile == m_game->m_allUnits[i]->m_coords)
		{
			return false;
		}
	}

	if (!IsInBounds(tile))
	{
		return false;
	}
	int value = m_heatMap[GetIndexFromGridCoord(tile)];

	//tile is in move range
	if (value <= m_def.m_movementRange && value != -1)
	{
		for (int i = 0; i < (int)g_theGame->m_allUnits.size(); i++)
		{
			if (g_theGame->m_allUnits[i] != this && g_theGame->m_allUnits[i]->m_coords == tile)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

bool Unit::CanAttackTile(IntVec2 const& tile) const
{
	if (!IsInBounds(tile))
	{
		return false;
	}
	int tileDistance = m_heatMap[GetIndexFromGridCoord(tile)];
	if (tileDistance == -1)
	{
		return false;
	}

	if (tileDistance <= m_def.m_attackRangeMax && tileDistance >= m_def.m_attackRangeMin)
	{
		for (int i = 0; i < (int)m_game->m_allUnits.size(); i++)
		{
			if (m_game->m_allUnits[i]->m_team != m_team && m_game->m_allUnits[i]->m_coords == tile)
			{
				return true;
			}
		}
	}
	return false;
}

void Unit::Die()
{
	ParticleEffect* explosion = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/ExplosionVaporum.xml", m_position + Vec3(0.f, 0.f, .5f), EulerAngles(), true, 2.f);
	explosion->SetScale(.1f);
	for (int i = 0; i < (int)m_game->m_allEntities.size(); i++)
	{
		if (m_game->m_allEntities[i] == this)
		{
			m_game->m_allEntities.erase(m_game->m_allEntities.begin() + i);
			break;
		}
	}

	for (int i = 0; i < (int)m_game->m_allUnits.size(); i++)
	{
		if (m_game->m_allUnits[i] == this)
		{
			m_game->m_allUnits.erase(m_game->m_allUnits.begin() + i);
			break;
		}
	}

	if (m_team == Team::RED)
	{
		for (int i = 0; i < (int)m_game->m_player2Units.size(); i++)
		{
			if (m_game->m_player2Units[i] == this)
			{
				m_game->m_player2Units.erase(m_game->m_player2Units.begin() + i);
				break;
			}
		}
	}

	else if (m_team == Team::BLUE)
	{
		for (int i = 0; i < (int)m_game->m_player1Units.size(); i++)
		{
			if (m_game->m_player1Units[i] == this)
			{
				m_game->m_player1Units.erase(m_game->m_player1Units.begin() + i);
				break;
			}
		}
	}
}

void Unit::UpdateHeatMap()
{
	std::vector<IntVec2> exploringQueue;
	for (int i = 0; i < (int)m_heatMap.size(); i++)
	{
		m_heatMap[i] = -1;
	}
	m_heatMap[GetIndexFromGridCoord(m_coords)] = 0;
	exploringQueue.push_back(m_coords);
	while (exploringQueue.size() > 0)
	{
		IntVec2 exploringCoords = exploringQueue[0];
		int exploringHeat = m_heatMap[GetIndexFromGridCoord(exploringCoords)];
		exploringQueue.erase(exploringQueue.begin());
		//add adjacent grid coords that have not been visited yet to exploring list
		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{
				//because hex grid
				if ((x == 1 && y == 1) || (y == -1 && x == -1))
				{
					continue;
				}
				IntVec2 nextGridCoords = exploringCoords + IntVec2(x, y);

				if (IsInBounds(nextGridCoords) && !IsBlocked(nextGridCoords))
				{
					//has not been touched yet, add to exploring queue
					if (m_heatMap[GetIndexFromGridCoord(nextGridCoords)] == -1)
					{
						m_heatMap[GetIndexFromGridCoord(nextGridCoords)] = exploringHeat + 1;
						exploringQueue.push_back(nextGridCoords);
					}
				}
			}
		}
	}
}

void Unit::AddVertsForReachableTiles(std::vector<Vertex_PCU>& verts) const
{
	UNUSED(verts);
	for (int i = 0; i < (int)m_heatMap.size(); i++)
	{
		if (CanMoveToTile(GetGridCoordFromIndex(i)))
		{
			g_theGame->AddVertsForHexCell(verts, GetGridCoordFromIndex(i), Rgba8(255, 255, 255, 100), false, 1.f);
		}
	}
}

void Unit::AddVertsForMovePath(std::vector<Vertex_PCU>& verts) const
{
	if (!CanMoveToTile(g_theGame->m_focusedHexCoords))
	{
		return;
	}
	std::vector<IntVec2> path;
	GetMovePath(path);
	for (int i = 0; i < (int)path.size(); i++)
	{
		g_theGame->AddVertsForHexCell(verts, path[i], Rgba8(255, 255, 255, 200), false, 1.f);
	}
}

void Unit::AddVertsForAttackableTiles(std::vector<Vertex_PCU>& verts) const
{
	for (int i = 0; i < (int)m_heatMap.size(); i++)
	{
		if (CanAttackTile(GetGridCoordFromIndex(i)))
		{
			g_theGame->AddVertsForHexCell(verts, GetGridCoordFromIndex(i), Rgba8(255, 0, 0, 200), false, .8f);
		}
	}
}

void Unit::GetMovePath(std::vector<IntVec2>& out_path) const
{
	IntVec2 currentCoords = g_theGame->m_focusedHexCoords;
	while (currentCoords != m_coords)
	{
		out_path.insert(out_path.begin(), currentCoords);
		int bestHeat = m_heatMap[GetIndexFromGridCoord(currentCoords)];
		IntVec2 bestCoords = currentCoords;
		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{
				//because hex grid
				if ((x == 1 && y == 1) || (y == -1 && x == -1))
				{
					continue;
				}
				IntVec2 compareCoords = IntVec2(currentCoords.x + x, currentCoords.y + y);
				if (!IsInBounds(compareCoords))
				{
					continue;
				}

				int compareHeat = m_heatMap[GetIndexFromGridCoord(compareCoords)];
				if (compareHeat < bestHeat && compareHeat != -1)
				{
					bestHeat = compareHeat;
					bestCoords = compareCoords;
				}
			}
		}
		if (bestCoords == currentCoords)
		{
			break;
		}

		currentCoords = bestCoords;
	}
	out_path.insert(out_path.begin(), currentCoords);
}

void Unit::Move()
{
	m_isMoving = true;
	m_moveStarted = false;
	m_currentMoveT = 0.f;
	m_goalCoords = g_theGame->m_focusedHexCoords;
	std::vector<IntVec2> movePath;
	GetMovePath(movePath);
	std::vector<Vec2> positions;
	for (int i = 0; i < (int)movePath.size(); i++)
	{
		positions.push_back(GetHexPosFromGridCoord(movePath[i]));
	}
	m_currentMoveCurve.SetPositions(positions);
	m_firstTilePos = positions[1];
	m_maxMoveT = (float)positions.size() - 1.f;
}

void Unit::Attack()
{
	m_isAttacking = true;
}

void Unit::UpdateMove(float deltaSeconds)
{
	m_previousFramePosition = m_position;
	m_currentMoveT += deltaSeconds * m_tileMoveSpeed;
	if (m_currentMoveT >= m_maxMoveT)
	{
		if (m_def.m_unitType == UnitType::ARTILERY)
		{
			m_actionsFinished = true;
			g_theGame->m_selectedUnit = nullptr;
		}
		m_isMoving = false;
		m_moveStarted = false;
		m_currentMoveT = 0.f;
		m_previousCoords = m_coords;
		m_previousTileRotation = m_orientationDegrees.m_yaw;
		m_coords = m_goalCoords;
		UpdateHeatMap();
		return;
	}

	//Vec2 newPos = Vec2::Lerp(GetHexPosFromGridCoord(m_coords), GetHexPosFromGridCoord(m_goalCoords), m_currentMoveT);
	Vec2 newPos = m_currentMoveCurve.EvaluateAtParametric(m_currentMoveT);
	m_position = Vec3(newPos.x, newPos.y, m_position.z);
	Vec2 facing = m_position.GetXY() - m_previousFramePosition.GetXY();

	if (facing.GetLength() > .0001f)
	{
		m_orientationDegrees.m_yaw = facing.GetOrientationDegrees();
	}


}
