#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/GameState.hpp"
#include "Game/Player.hpp"

Entity::Entity(GameState* gameState, EntityType entityType, const Vec2& startPos, EntityConfig const& config)
	: m_gameState(gameState)
	, m_entityType(entityType)
	, m_position(startPos)
	, m_animationTimer(Timer(1.f, g_theApp->GetGameClock()))
	, m_hideTimer(Timer(-1.f, g_theApp->GetGameClock()))
	, m_becomeHazardTimer(Timer(-1.f, g_theApp->GetGameClock()))
	, m_liveTimer(Timer(-1.f, g_theApp->GetGameClock()))
	, m_config(config)
{
	if (!m_config.m_useConfig)
	{
		return;
	}

	m_orientationDegrees = m_config.m_startOrientaiton;
	m_texture = m_config.m_texture;
	m_spriteBounds = m_config.m_spriteBounds;
	m_uvs = m_config.m_uvs;
	m_normalizedPivot = m_config.m_normalizedPivot;
	m_color = m_config.m_startColor;
	m_sortOrder = m_config.m_sortOrder;

	m_isHazard = m_config.m_startHazard;
	m_velocity = m_config.m_startVelocity;
	m_acceleration = m_config.m_startAcceleration;
	m_rotationSpeed = m_config.m_rotationSpeed;
	m_radius = m_config.m_collisionRadius;
	m_simulatePhysics = m_config.m_simulatePhysics;

	m_liveTimer.m_period = m_config.m_liveTime;
	m_becomeHazardTimer.m_period = m_config.m_becomeHazardTime;
	m_hideTimer.m_period = m_config.m_hideTime;

	if (m_config.m_hideTime > 0.f)
	{
		m_hideTimer.Start();
	}
	else
	{
		if (m_config.m_startAnimation != "")
		{
			PlayAnimation(m_config.m_startAnimation);
		}
		if (m_config.m_liveTime > 0.f)
		{
			m_liveTimer.Start();
		}
		if (m_becomeHazardTimer.m_period > 0.f)
		{
			m_becomeHazardTimer.Start();
		}
	}
}

Entity::~Entity()
{
	
}

void Entity::Update(float deltaSeconds)
{
	if (!m_hideTimer.IsStopped() && !m_hideTimer.HasPeriodElapsed())
	{
		return;
	}
	else if (m_hideTimer.HasPeriodElapsed())
	{
		m_hideTimer.Stop();
		if (m_liveTimer.m_period > 0)
		{
			m_liveTimer.Start();
		}
		if (m_becomeHazardTimer.m_period > 0)
		{
			m_becomeHazardTimer.Start();
		}
		if (m_config.m_useConfig && m_config.m_startAnimation != "")
		{
			PlayAnimation(m_config.m_startAnimation);
		}
	}
	if (!m_becomeHazardTimer.IsStopped() && m_becomeHazardTimer.HasPeriodElapsed())
	{
		m_becomeHazardTimer.Stop();
		m_isHazard = true;
	}
	if (!m_liveTimer.IsStopped() && m_liveTimer.HasPeriodElapsed())
	{
		DestroyEntity();
		return;
	}
	UpdateAnimation();
	UpdatePhysics(deltaSeconds);
	UpdateChildren(deltaSeconds);
}

void Entity::Render()
{
	if (!m_hideTimer.IsStopped() && !m_hideTimer.HasPeriodElapsed())
	{
		return;
	}

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	Mat44 modelMatrix = GetModelMatrix();

	if (m_parent != nullptr)
	{
		g_theRenderer->SetModelConstants(modelMatrix, m_parent->m_color);
	}
	else
	{
		g_theRenderer->SetModelConstants(modelMatrix, m_color);
	}
	g_theRenderer->BindTexture(m_texture);
	std::vector<Vertex_PCU> entityVerts;
	Vec2 spriteDimensions = m_spriteBounds.GetDimensions();
	AABB2 centeredSpriteBounds(m_spriteBounds.m_mins - spriteDimensions * .5f, m_spriteBounds.m_maxs - spriteDimensions * .5f);
	AddVertsForAABB2D(entityVerts, centeredSpriteBounds, Rgba8::WHITE, m_uvs.m_mins, m_uvs.m_maxs, (float)m_sortOrder / 10.f);
	g_theRenderer->DrawVertexArray(entityVerts.size(), entityVerts.data());
	RenderChildren();
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees(m_orientationDegrees);
}

bool Entity::IsScreenSpace()
{
	if (m_entityType == EntityType::BUTTON)
	{
		return true;
	}
	return false;
}

void Entity::UpdatePhysics(float deltaSeconds)
{
	if (!m_simulatePhysics)
	{
		return;
	}
	if (m_drag > 0.f)
	{
		AddForce(-m_velocity * m_drag);
		m_velocity += m_acceleration * deltaSeconds;
		m_position += m_velocity * deltaSeconds;
		m_acceleration = Vec2::ZERO;
	}
	else
	{
		m_orientationDegrees += deltaSeconds * m_rotationSpeed;
		m_velocity += m_acceleration * deltaSeconds;
		m_position += m_velocity * deltaSeconds;
	}
}

void Entity::SetTimeOffsetInAnimation(float timeOffset)
{
	m_animationTimer.SetTimePosition(timeOffset);
}

void Entity::AddForce(Vec2 const& force)
{
	m_acceleration += force;
}

void Entity::AddImpulse(Vec2 const& impulse)
{
	m_velocity += impulse;
}

void Entity::PlayAnimation(std::string animationName)
{
	AnimationDefinition const* animDef = AnimationDefinition::GetByName(animationName);
	if (animDef == nullptr)
	{
		ERROR_RECOVERABLE("Tried to play an animation that does not exist");
		return;
	}
	m_currentAnimation = animDef;
	m_animationTimer.m_period = animDef->GetAnimationLength();
	m_animationTimer.Start();
}

void Entity::DestroyEntity()
{
	m_isGarbage = true;
}

Vec2 Entity::GetPosition()
{
	return m_position + m_localPosition;
}

bool Entity::OverlapsPlayer(Player* player)
{
	if (!m_hideTimer.IsStopped() && !m_hideTimer.HasPeriodElapsed())
	{
		return false;
	}
	return DoDiscsOverlap(GetPosition(), m_radius * m_scale.x, player->GetPosition(), player->m_radius);
}

void Entity::SetPosition(Vec2 const& pos)
{
	m_position = pos;
}

void Entity::AddChildEntity(Entity* childToAdd)
{
	m_children.push_back(childToAdd);
	childToAdd->m_parent = this;
}

Mat44 Entity::GetModelMatrix()
{
	Mat44 modelMatrix;
	if (m_parent != nullptr)
	{
		modelMatrix = m_parent->GetModelMatrix();
	}

	modelMatrix.AppendTranslation2D(GetPosition());
	modelMatrix.AppendZRotation(m_orientationDegrees);
	Vec2 pivotPos;
	Vec2 spriteDimensions = m_spriteBounds.GetDimensions();
	pivotPos.x = Lerp(-spriteDimensions.x * .5f, spriteDimensions.x * .5f, m_normalizedPivot.x);
	pivotPos.y = Lerp(-spriteDimensions.y * .5f, spriteDimensions.y * .5f, m_normalizedPivot.y);
	modelMatrix.AppendTranslation2D(-pivotPos);
	modelMatrix.AppendScaleNonUniform2D(m_scale);

	return modelMatrix;
}

void Entity::UpdateChildren(float deltaSeconds)
{
	for (int i = 0; i < (int)m_children.size(); i++)
	{
		m_children[i]->Update(deltaSeconds);
	}
}

void Entity::RenderChildren()
{
	for (int i = 0; i < (int)m_children.size(); i++)
	{
		m_children[i]->Render();
	}
}

void Entity::UpdateAnimation()
{
	if (m_currentAnimation == nullptr || g_theApp->m_isPaused)
	{
		return;
	}
	if (m_animationTimer.HasPeriodElapsed())
	{
		if (m_currentAnimation->m_playbackMode == PlaybackMode::ONCE)
		{
			//set all values to how they should be at the end of the animaiton
			SetValuesToEndOfAnimation();
			m_currentAnimation = nullptr;
			return;
		}
		if (m_currentAnimation->m_playbackMode == PlaybackMode::LOOP)
		{
			m_animationTimer.Start();
		}
	}
	float currT = m_animationTimer.GetElapsedTime();
	for (int i = 0; i < (int)m_currentAnimation->m_tracks.size(); i++)
	{
		Track const& currTrack = m_currentAnimation->m_tracks[i];
		for (int k = 0; k < (int)currTrack.m_keyframes.size() - 1; k++)
		{
			Keyframe const& currKeyframe = currTrack.m_keyframes[k];
			Keyframe const& nextKeyFrame = currTrack.m_keyframes[k+1];
			if (currT >= currKeyframe.m_time && currT < nextKeyFrame.m_time)
			{
				float normalizedT = GetFractionWithinRange(currT, currKeyframe.m_time, nextKeyFrame.m_time);
				float easingValue = currKeyframe.GetEasingOutput(normalizedT);
				if (currKeyframe.m_changeColor)
				{
					m_color = LerpColor(currKeyframe.m_color, nextKeyFrame.m_color, easingValue);
				}
				if (currKeyframe.m_changeLocalPosition)
				{
					m_localPosition = Vec2::Lerp(currKeyframe.m_localPosition, nextKeyFrame.m_localPosition, easingValue);
				}
				if (currKeyframe.m_changeScale)
				{
					m_scale = Vec2::Lerp(currKeyframe.m_scale, nextKeyFrame.m_scale, easingValue);
				}
				if (currKeyframe.m_changeRotation)
				{
					m_orientationDegrees = Lerp(currKeyframe.m_rotation, nextKeyFrame.m_rotation, easingValue);
				}
				if (currKeyframe.m_changeSprite)
				{
					int spriteIdx = currTrack.m_spriteSheet->GetIndexFromCoords(currKeyframe.m_spriteCoords);
					SpriteDefinition const& spriteDef = currTrack.m_spriteSheet->GetSpriteDef(spriteIdx);
					m_texture = spriteDef.GetTexture();
					m_uvs = spriteDef.GetUVs();
				}
			}
		}
	}
}

void Entity::SetValuesToEndOfAnimation()
{
	for (int i = 0; i < (int)m_currentAnimation->m_tracks.size(); i++)
	{
		Track const& currTrack = m_currentAnimation->m_tracks[i];
		Keyframe const& lastKeyFrame = currTrack.m_keyframes[currTrack.m_keyframes.size() - 1];
		if (lastKeyFrame.m_changeColor)
		{
			m_color = lastKeyFrame.m_color;
		}
		if (lastKeyFrame.m_changeLocalPosition)
		{
			m_localPosition = lastKeyFrame.m_localPosition;
		}
		if (lastKeyFrame.m_changeScale)
		{
			m_scale = lastKeyFrame.m_scale;
		}
		if (lastKeyFrame.m_changeRotation)
		{
			m_orientationDegrees = lastKeyFrame.m_rotation;
		}
		if (lastKeyFrame.m_changeSprite)
		{
			int spriteIdx = currTrack.m_spriteSheet->GetIndexFromCoords(lastKeyFrame.m_spriteCoords);
			SpriteDefinition const& spriteDef = currTrack.m_spriteSheet->GetSpriteDef(spriteIdx);
			m_texture = spriteDef.GetTexture();
			m_uvs = spriteDef.GetUVs();
		}
	}

}

