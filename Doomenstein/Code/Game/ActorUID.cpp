#include "Game/ActorUID.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

const ActorUID ActorUID::INVALID;

ActorUID::ActorUID(unsigned int salt, unsigned int index)
{
	
	m_data = 0x00000000;
	salt &= 0x0000ffff;
	index &= 0x0000ffff;
	m_data = (salt << 16) | index;
}

bool ActorUID::IsValid() const
{
	return *this != INVALID;
}

Actor* ActorUID::GetActor()
{
	return g_theGame->m_currentMap->GetActorByUID(*this);
}

unsigned int ActorUID::GetIndex() const
{
	return m_data & 0x0000ffff;
}

unsigned int ActorUID::GetSalt() const
{
	unsigned int salt = (m_data & 0xffff0000) >> 16;
	return salt;
}

bool ActorUID::operator==(const ActorUID& other) const
{
	return other.m_data == m_data;
}

bool ActorUID::operator!=(const ActorUID& other) const
{
	return other.m_data != m_data;
}
