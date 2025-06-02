#pragma once
#include "Game/Entity.hpp"

class Texture;
class Material;

class Prop : public Entity
{
public:
	Prop(Vec3 const& startPos, EulerAngles const& startOrientation = EulerAngles());
	virtual ~Prop();
	void Update(float deltaSeconds) override;
	void Render() const override;
	void RenderUnlit() const;

	void CreateCube(float sideLength = 1.f);
	void CreateSphere(float radius = 1.f);
	void CreateQuad();
	void CreateGrid();
	bool m_unlit = false;
	Material* m_material = nullptr;

protected:
	void CreateBuffers();


	std::vector<Vertex_PCU> m_unlitVertexes;
	Texture* m_unlitTexture = nullptr;

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;

};