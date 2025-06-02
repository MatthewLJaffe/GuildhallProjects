#include "Game/Prop.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Game/Game.hpp"

Prop::Prop(Vec3 const& startPos, EulerAngles const& startOrientation)
	: Entity(startPos, startOrientation)
{
}

Prop::~Prop()
{
	delete m_vertexBuffer;
	delete m_indexBuffer;
	delete m_debugVertexBuffer;
	delete m_material;
}

void Prop::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Prop::Render() const
{
	if (m_unlit)
	{
		RenderUnlit();
		return;
	}

	g_theRenderer->SetModelConstants(GetWorldTransform(), m_color);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->BindMaterial(m_material);
	g_theRenderer->DrawVertexBufferIndexed(m_vertexBuffer, m_indexBuffer, (int)m_indexes.size(), VertexType::VERTEX_TYPE_PCUTBN);
}

void Prop::RenderUnlit() const
{
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants(GetWorldTransform(), m_color);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(m_unlitTexture);
	g_theRenderer->DrawVertexArray(m_unlitVertexes.size(), m_unlitVertexes.data());
}

void Prop::CreateCube(float sideLength)
{
	m_unlitVertexes.clear();
	m_debugVertexes.clear();
	m_vertexes.clear();
	m_indexes.clear();

	Vec3 bottomLeftNear = Vec3(-.5f, .5f, -.5f) * sideLength;
	Vec3 bottomRightNear = Vec3(-.5f, -.5f, -.5f) * sideLength;
	Vec3 topRightNear = Vec3(-.5f, -.5f, .5f) * sideLength;
	Vec3 topLeftNear = Vec3(-.5f, .5f, .5f) * sideLength;

	Vec3 bottomLeftFar = Vec3(.5f, .5f, -.5f) * sideLength;
	Vec3 bottomRightFar = Vec3(.5f, -.5f, -.5f) * sideLength;
	Vec3 topRightFar = Vec3(.5f, -.5f, .5f) * sideLength;
	Vec3 topLeftFar = Vec3(.5f, .5f, .5f) * sideLength;

	AddVertsForQuad3D(m_vertexes, bottomRightFar, bottomLeftFar, topLeftFar, topRightFar);
	AddVertsForQuad3D(m_vertexes, bottomLeftNear, bottomRightNear, topRightNear, topLeftNear);

	AddVertsForQuad3D(m_vertexes, topLeftNear, topRightNear, topRightFar, topLeftFar);
	AddVertsForQuad3D(m_vertexes, bottomRightNear, bottomLeftNear, bottomLeftFar, bottomRightFar);

	AddVertsForQuad3D(m_vertexes, bottomLeftFar, bottomLeftNear, topLeftNear, topLeftFar);
	AddVertsForQuad3D(m_vertexes, bottomRightNear, bottomRightFar, topRightFar, topRightNear);

	for (int i = 0; i < m_vertexes.size(); i++)
	{
		m_indexes.push_back((unsigned int)i);
	}

	CreateBuffers();
	CreateDebugTangentAndBasisVectors();
}

void Prop::CreateSphere(float radius)
{
	m_vertexes.clear();
	m_indexes.clear();
	AddVertsForSphere3D(m_vertexes, Vec3::ZERO, radius);

	for (int i = 0; i < m_vertexes.size(); i++)
	{
		m_indexes.push_back((unsigned int)i);
	}
	CreateBuffers();
	CreateDebugTangentAndBasisVectors();

}

void Prop::CreateQuad()
{
	m_unlitVertexes.clear();
	m_debugVertexes.clear();
	m_vertexes.clear();
	m_indexes.clear();

	m_unlitVertexes.push_back(Vertex_PCU(Vec3(0.f, -.5f, -.5f), m_color, Vec2::ZERO));
	m_unlitVertexes.push_back(Vertex_PCU(Vec3(0.f, .5f, -.5f), m_color, Vec2(1.f, 0.f)));
	m_unlitVertexes.push_back(Vertex_PCU(Vec3(0.f, .5f, .5f), m_color, Vec2(1.f, 1.f)));
	m_unlitVertexes.push_back(Vertex_PCU(Vec3(0.f, -.5f, .5f), m_color, Vec2(0.f, 1.f)));


	m_vertexes.push_back(Vertex_PCUTBN(Vec3(0.f, -.5f, -.5f), m_color, Vec2::ZERO, Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f)));
	m_vertexes.push_back(Vertex_PCUTBN(Vec3(0.f, .5f, -.5f), m_color, Vec2(1.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f)));
	m_vertexes.push_back(Vertex_PCUTBN(Vec3(0.f, .5f, .5f), m_color, Vec2(1.f, 1.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f)));
	m_vertexes.push_back(Vertex_PCUTBN(Vec3(0.f, -.5f, .5f), m_color, Vec2(0.f, 1.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f)));

	m_indexes.push_back(0);
	m_indexes.push_back(1);
	m_indexes.push_back(2);
	m_indexes.push_back(0);
	m_indexes.push_back(2);
	m_indexes.push_back(3);

	CreateBuffers();
	CreateDebugTangentAndBasisVectors();
}

void Prop::CreateGrid()
{
	AABB3 gridLineBounds;
	gridLineBounds.m_mins.y = -50.f;
	gridLineBounds.m_maxs.y = 50.f;
	float defaultLineThickness = .025f;
	for (int x = -50; x < 50; x++)
	{
		Rgba8 color(100, 100, 100);
		float lineThickness = defaultLineThickness;
		if (x % 5 == 0)
		{
			color = Rgba8::GREEN;
			lineThickness = defaultLineThickness * 2.f;
		}
		gridLineBounds.m_mins.z = -lineThickness;
		gridLineBounds.m_maxs.z = lineThickness;

		gridLineBounds.m_mins.x = static_cast<float>(x) - lineThickness;
		gridLineBounds.m_maxs.x = static_cast<float>(x) + lineThickness;
		AddVertsForAABB3D(m_unlitVertexes, gridLineBounds, color);
	}

	gridLineBounds.m_mins.x = -50.f;
	gridLineBounds.m_maxs.x = 50.f;
	for (int y = -50; y < 50; y++)
	{
		Rgba8 color(100, 100, 100);
		float lineThickness = defaultLineThickness;
		if (y % 5 == 0)
		{
			color = Rgba8::RED;
			lineThickness = defaultLineThickness * 2.f;
		}
		gridLineBounds.m_mins.z = -lineThickness;
		gridLineBounds.m_maxs.z = lineThickness;

		gridLineBounds.m_mins.y = static_cast<float>(y) - lineThickness;
		gridLineBounds.m_maxs.y = static_cast<float>(y) + lineThickness;
		AddVertsForAABB3D(m_unlitVertexes, gridLineBounds, color);
	}
	m_unlit = true;
}

void Prop::CreateBuffers()
{
	if (m_vertexBuffer != nullptr)
	{
		delete m_vertexBuffer;
	}
	if (m_indexBuffer != nullptr)
	{
		delete m_indexBuffer;
	}
	if (m_debugVertexes.size() > 0)
	{
		m_debugVertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * m_debugVertexes.size());
		g_theRenderer->CopyCPUToGPU((void*)m_debugVertexes.data(), m_debugVertexes.size(), m_debugVertexBuffer);
	}

	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN) * 4);
	g_theRenderer->CopyCPUToGPU((void*)m_vertexes.data(), sizeof(Vertex_PCUTBN) * m_vertexes.size(), m_vertexBuffer);

	m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int) * 6);
	g_theRenderer->CopyCPUToGPU((void*)m_indexes.data(), sizeof(unsigned int) * m_indexes.size(), m_indexBuffer);
}

