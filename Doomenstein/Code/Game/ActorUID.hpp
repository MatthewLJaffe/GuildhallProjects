#pragma once
class Actor;

struct ActorUID
{
public:
	ActorUID() = default;
	ActorUID(unsigned int salt, unsigned int index);

	bool IsValid() const;
	Actor* GetActor();
	unsigned int GetIndex() const;
	unsigned int GetSalt() const;
	bool operator==(const ActorUID& other) const;
	bool operator!=(const ActorUID& other) const;

	static const ActorUID INVALID;

private:
	unsigned int m_data = 0;
};