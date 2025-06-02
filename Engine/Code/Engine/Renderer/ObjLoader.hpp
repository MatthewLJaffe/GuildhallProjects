#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Renderer/MtlLib.hpp"
#include <unordered_map>

class Renderer;

struct VertexHash
{
	std::size_t operator()(const IntVec3& k) const
	{
		return ((size_t)k.x) ^ ((size_t)k.y << 1) ^ ((size_t)k.z << 2);
	}
};

struct VertexEqual
{
	bool operator()(const IntVec3& lhs, const IntVec3& rhs) const
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}
};

struct Face
{
	std::vector<IntVec3> m_vertexes;

	Face(std::string const& faceText, Mtl mtl);
	std::vector<IntVec3> GetOrderedVertexes();
	std::vector<unsigned int> GetOrderedIndexes();
	void AddFace(std::vector<Vertex_PCUTBN>& out_vertexes, std::vector<unsigned int>& out_indexes, std::vector<Vec3> const& positions, std::vector<Vec2> const& uvs, std::vector<Vec3> const& normals, 
		std::unordered_map<IntVec3, int, VertexHash, VertexEqual>& vertHashMap);


	Mtl m_mtl;
};

class ObjLoader
{
public:
	static bool Load(Renderer* renderer, const std::string& fileName,
		std::vector<Vertex_PCUTBN>& outVertexes, std::vector<unsigned int>& outIndexes,
		bool& outHasNormals, bool& outHasUVs, const Mat44& transform = Mat44());
	static void CalculateTangentAndBasisVectors(std::vector<Vertex_PCUTBN>& outVertexes, std::vector<unsigned int>& indexes, bool computeNormals, bool computeTangents = true);
	static void ComputeTBNForVertex(Vertex_PCUTBN& vert0, Vertex_PCUTBN& vert1, Vertex_PCUTBN& vert2, bool computeNormals, bool computeTangents = true);
};

