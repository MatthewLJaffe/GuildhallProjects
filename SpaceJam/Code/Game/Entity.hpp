#pragma once
#include "Game/GameCommon.hpp"

class Game;
class App;
class Renderer;

extern App* g_theApp;
extern Renderer* g_theRenderer;

class Entity
{

public:
	Entity(Vec3 const& startPos, EulerAngles const& startOrientation = EulerAngles());
	Entity(Vec3 const& startPos, Mat44 const& startOrientation);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;

	virtual Mat44 GetWorldTransform() const;
	Vec3 GetWorldPosition() const;
	Vec3 GetWorldScale() const;
	Vec3 GetLocalPosition() const;
	void SetLocalPosition( Vec3 const& newLocalPos);
	Mat44 GetWorldOrientaiton() const;
	Mat44 GetLocalOrientation() const;
	void SetLocalOrientation(EulerAngles const& newLocalOrientation);
	void SetLocalOrientation(Mat44 const& newLocalOrientation);
	void SetLocalScale(Vec3 const& scale);
	Vec3 GetForwardNormal();
	void Translate(Vec3 const& disp);

	std::vector<Entity*> m_children;
	Entity* m_parent = nullptr;
	void AddChild(Entity* child);

public:
	Vec3 m_velocity = Vec3(0.f, 0.f, 0.f);
	Rgba8 m_color = Rgba8::WHITE;
	bool m_isAlive = true;
protected:
	Mat44 m_transform;
	void CreateDebugTangentAndBasisVectors();
	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	std::vector<Vertex_PCU> m_debugVertexes;
	VertexBuffer* m_debugVertexBuffer = nullptr;
};
