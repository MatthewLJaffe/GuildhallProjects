#pragma once
#include "Game/VisualTest.hpp"
#include "Engine/Core/FileUtils.hpp"

struct BoundingDisc2D
{
	float m_radius;
	Vec2 m_centerPosition;
};

struct Ray2D
{
	Vec2 m_start;
	Vec2 m_normal;
	float m_distance = 0.f;
	unsigned long long m_arrowMask = 0;
};

class ConvexPolySceneObject
{
public:
	ConvexPolySceneObject(std::vector<Vec2> vertexes, BoundingDisc2D boundingDisc, bool loadConvexHull = true);
	void AddVertsForConvexPoly(std::vector<Vertex_PCU>& verts, Rgba8 color = Rgba8::WHITE);
	void AddVertsForConvexPolyEdges(std::vector<Vertex_PCU>& verts, float lineThickness, Rgba8 color = Rgba8::WHITE);

	BoundingDisc2D m_boundingDisc;
	ConvexHull2D m_convexHull;
	ConvexPoly2D m_convexPoly;
	unsigned long long m_sceneMask = 0;
};

struct GHCSChunk
{
	uint8_t m_chunkType = 0;
	uint32_t m_chunkHeaderStart = 0;
	uint32_t m_chunkTotalSize = 0;
};

constexpr unsigned int headerTOCPtrOffset = 8;



class ConvexSceneEditorTest : public VisualTest
{
public:
	ConvexSceneEditorTest(VisualTestType myTestType, Game* game);
	~ConvexSceneEditorTest() = default;
	void Update(float deltaSeconds) override;
	void Render() override;
	void InitializeTest() override;
	void RandomizeTest() override;
	void AddVertsForHullPlanesAndRayInterceptPoints(std::vector<Vertex_PCU>& verts);
	void HalveScene();
	void DoubleScene();
	void AddRandomConvexObject();
	void AddRandomInvisibleRay();
	void TestInvisibleRays();
	void DoubleRays();
	void HalfRays();
	void RotateObject(float theta);
	void ScaleObject(float scaleFactor);
	void MoveObjectWithMouse();
	void CreateRayMask(Ray2D& rayToMask);
	void ShittyRaycastVsGrid(std::vector<unsigned long long>& cellsHit, Vec2 const& rayStartPos, Vec2 const& rayNormal,float rayDistance);
	void ImprovedRaycastVsGrid(std::vector<unsigned long long>& cellsHit, Vec2 const& rayStartPos, Vec2 const& rayNormal, float rayDistance);
	void ComputeBoundingDisc(ConvexPolySceneObject& sceneObject);

	void CreateSceneObjectMask(ConvexPolySceneObject& sceneObject);
	void AddVertsForGridLines(std::vector<Vertex_PCU>& verts);
	void AddVertsForBoundingDiscs(std::vector<Vertex_PCU>& verts);
	unsigned long long  GetGridNumber(Vec2 const& pos);
	void AppendTestFileBufferData(BufferWriter& bufferWriter, BufferEndianMode endianMode);
	void ParseTestFileBufferData(BufferParser& bufferParser, BufferEndianMode endianMode);
	void TestBinaryFileLoad();
	void TestBinaryFileSave();
	void WriteOutGHCS(BufferWriter& bufferWriter, bool skipDisk, bool skipConvexHull, bool skipBitBucket);
	void WriteOutChunk(BufferWriter& bufferWriter, uint8_t chunkType);
	void LoadGHCS(BufferParser& bufferParser);
	void LoadChunk(BufferParser& bufferParser, bool boundingDiscChunkPresent, bool bitBucketChunkPresent, bool convexHullChunkPresent);

	static bool Event_SaveConvexScene(EventArgs& args);
	static bool Event_LoadConvexScene(EventArgs& args);

	void AddConvexObjectFromVerts(std::vector<Vec2> const& verts, bool boundingDiscChunkPresent, bool bitBucketChunkPresent, bool conevxHullChunkPresent);


	std::vector<GHCSChunk> m_loadedChunks;
	Vec2 m_mousePos;
	Vec2 m_prevMousePos;
	int m_gridWidth = 8;
	int m_gridHeight = 8;
	float m_gridCellWidth = 200.f / (float)m_gridWidth;
	float m_gridCellHeight = 100.f / (float)m_gridHeight;
	int m_gridCells = m_gridHeight * m_gridWidth;
	std::vector<ConvexPolySceneObject> m_sceneObjects;
	std::vector<Ray2D> m_invisibleRays;
	int m_numObjects = 1;
	int m_hilightedObjectIndex = -1;
	int m_numInvisibleRaycasts = 8192;
	Ray2D m_controlledRay;
	RaycastResult2D m_closestRaycast;
	int m_objectHitIndex = -1;
	float m_startMoveSpeed = 25.f;
	float m_arrowTranslateSpeed = 25.f;
	std::vector<uint8_t> m_sceneData;

	bool m_useBitBuckes = true;
	bool m_useBoundingDisc = true;
	bool m_translucentDraw = false;
	bool m_debugDrawDiscs = false;
	bool m_debugDrawPartitioning = false;
};

