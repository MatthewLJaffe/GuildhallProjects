#include "Game/Thrall.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/GameScene.hpp"

Thrall::Thrall(SpawnInfo const& spawnInfo, ActorUID actorUID)
	: Actor(spawnInfo, actorUID)
{
	m_explodeTimer = Timer(BEAT_TIME, g_theApp->m_clock);
}

void Thrall::Update(float deltaSeconds)
{
	if (m_deathTimer.HasPeriodElapsed())
	{
		m_isAlive = false;
		return;
	}

	//AI
	if (m_currentBehaviour != nullptr)
	{
		AIBehaviour* prevBehaviour = m_currentBehaviour;
		m_currentBehaviour = m_currentBehaviour->Tick(deltaSeconds);
		if (prevBehaviour != m_currentBehaviour)
		{
			prevBehaviour->OnEnd();
			m_currentBehaviour->OnBegin();
		}
	}
	if (m_velocity.GetLengthSquared() > 0.f)
	{
		m_velocity = m_velocity.Clamp(m_definition->m_maxSpeed);
		Mat44 rotation;
		Vec3 iBasis = -m_velocity.GetNormalized();
		Vec3 kBasis = g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D().GetNormalized();
		Vec3 jBasis = CrossProduct3D(kBasis, iBasis).GetNormalized();
		rotation.SetIJK3D(iBasis, jBasis, kBasis);
		SetLocalOrientation(rotation);
	}

	Translate(m_velocity * deltaSeconds);
	/*
	float distanceToTarget = GetDistance3D(targetPos, GetWorldPosition());
	if (!m_exploding)
	{
		if (distanceToTarget < m_maxExplodeDistance && g_theGame->m_playerShip->m_isAlive)
		{
			m_exploding = true;
			float timeToNextBeat = BEAT_TIME - g_theGame->m_gameScene->GetCurrentTimeFromLastBeat();
			m_explodeTimer.m_period = timeToNextBeat + BEAT_TIME;
			m_explodeMidwayPoint = timeToNextBeat / m_explodeTimer.m_period;
			m_explodeTimer.Start();
		}
	}
	if (m_exploding)
	{
		float m_maxScale = 1.75f;
		//#ToDo explode animation
		if (m_explodeTimer.GetElapsedFraction() < m_explodeMidwayPoint)
		{
			float t = m_explodeTimer.GetElapsedFraction() / m_explodeMidwayPoint;
			t = SmoothStart5(t);
			SetLocalScale(Vec3::Lerp(Vec3(1.f, 1.f, 1.f), Vec3(m_maxScale, m_maxScale, m_maxScale), t));
			//m_color = LerpColor(Rgba8::WHITE, Rgba8::YELLOW, t);
		}
		else
		{
			float t = GetFractionWithinRange(m_explodeTimer.GetElapsedFraction(), m_explodeMidwayPoint, 1.f);
			SetLocalScale(Vec3::Lerp(Vec3(m_maxScale, m_maxScale, m_maxScale), Vec3(1.f, 1.f, 1.f), t));
			t = SmoothStop3(t);
			m_color = LerpColor(Rgba8::WHITE, Rgba8::RED, t);
		}

		if (m_explodeTimer.HasPeriodElapsed() || m_explodeTimer.GetElapsedFraction() > .9f && g_theGame->m_gameScene->m_isFrameBeat)
		{
			Die();
		}
	}
	*/
}

void Thrall::Die()
{
	Actor::Die();
	//g_theGame->m_gameScene->SpawnThrallClose();
}

/*
Vec3 Thrall::ComputeChaseDirection(float deltaSeconds, float weight)
{
	UNUSED(deltaSeconds);
	Vec3 chaseDir = Vec3::ZERO;
	//slow down
	Vec3 targetPos = g_theGame->m_playerShip->GetWorldPosition() + g_theGame->m_playerShip->GetForwardNormal();
	
	float distanceToTarget = GetDistance3D(targetPos, GetWorldPosition());
	if (distanceToTarget < m_maxChaseDistance)
	{
		if (m_velocity.GetLength() > .1f)
		{
			chaseDir = -m_velocity.GetNormalized();
			chaseDir.Clamp(m_velocity.GetLength() / (weight * m_definition->m_acceleration * deltaSeconds));
		}
	}
	//chase
	else
	{
		chaseDir = (targetPos - GetWorldPosition()).GetNormalized();
	}
	return chaseDir * weight;
}

Vec3 Thrall::ComputeSeperationDirection(float weight)
{
	Vec3 seperateDir = Vec3::ZERO;
	std::vector<Actor*> const& thralls = g_theGame->m_gameScene->m_thralls;
	float totalWeight = 0.f;
	for (int i = 0; i < (int)thralls.size(); i++)
	{
		if (!IsValidActor(thralls[i]) || thralls[i] == this)
		{
			continue;
		}
		Vec3 otherThrallPos = thralls[i]->GetWorldPosition();
		float maxAvoidDistance = 12.5f;
		float minAvoidWeight = .0f;
		float maxAvoidWeight = 1.f;

		if (GetDistanceSquared3D(otherThrallPos, GetWorldPosition()) > maxAvoidDistance * maxAvoidDistance)
		{
			continue;
		}
		float normalizedDistance = GetDistance3D(otherThrallPos, GetWorldPosition()) / maxAvoidDistance;
		float distanceWeight = 1.f - normalizedDistance;

		//prioritize close
		distanceWeight = SmoothStart2(distanceWeight);
		distanceWeight = Lerp(minAvoidWeight, maxAvoidWeight, distanceWeight);

		if (distanceWeight < minAvoidWeight)
		{
			distanceWeight = minAvoidWeight;
		}
		seperateDir += distanceWeight * (GetWorldPosition() - otherThrallPos).GetNormalized();
		totalWeight += distanceWeight;
	}
	if (totalWeight > 0.f)
	{
		return seperateDir * (weight / (float)totalWeight);
	}
	return Vec3::ZERO;
}
*/