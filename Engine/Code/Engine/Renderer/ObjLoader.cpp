#include "Engine/Renderer/ObjLoader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/MtlLib.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time.hpp"

Face::Face(std::string const& faceText, Mtl mtl)
    : m_mtl(mtl)
{
    Strings faceStrings = SplitStringOnDelimiter(faceText, ' ', true);
    for (int i = 0; i < (int)faceStrings.size(); i++)
    {
		IntVec3 faceIndexes;
        if (SplitStringOnDelimiter(faceStrings[i], '/', true).size() == 0)
        {
            faceIndexes.x = atoi(faceStrings[i].c_str());
			faceIndexes.y = 0;
			faceIndexes.z = 0;
			m_vertexes.push_back(faceIndexes);
        }
        else
        {
			faceIndexes.SetFromText(faceStrings[i].c_str(), '/');
			m_vertexes.push_back(faceIndexes);
        }
    }
}

bool ObjLoader::Load(Renderer* renderer, const std::string& fileName, std::vector<Vertex_PCUTBN>& outVertexes, 
    std::vector<unsigned int>& outIndexes, bool& outHasNormals, bool& outHasUVs, const Mat44& transform)
{
    std::string objFilePath= fileName;
    Mat44 modelTransform = transform;
    if (SplitStringOnDelimiter(fileName, '.')[1] == "xml")
    {
		XmlDocument xmlDocument;
		GUARANTEE_OR_DIE(xmlDocument.LoadFile(fileName.c_str()) == 0, std::string("Failed to load ") + fileName);
		XmlElement* rootElement = xmlDocument.FirstChildElement("Model");
		GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the Model from ") + fileName);
		std::string name = ParseXmlAttribute(*rootElement, "name", "Missing");
		objFilePath = ParseXmlAttribute(*rootElement, "path", "Missing");
		std::string materialFilePath = ParseXmlAttribute(*rootElement, "material", "Missing");
		XmlElement* transformElement = rootElement->FirstChildElement("Transform");
		Vec3 x = ParseXmlAttribute(*transformElement, "x", Vec3::ZERO);
		Vec3 y = ParseXmlAttribute(*transformElement, "y", Vec3::ZERO);
		Vec3 z = ParseXmlAttribute(*transformElement, "z", Vec3::ZERO);
		Vec3 t = ParseXmlAttribute(*transformElement, "t", Vec3::ZERO);
		float scale = ParseXmlAttribute(*transformElement, "scale", 1.f);
        modelTransform = Mat44();
        modelTransform.SetIJKT3D(x, y, z, t);
        modelTransform.AppendScaleUniform3D(scale);
    }
    else if (SplitStringOnDelimiter(fileName, '.')[1] != "obj")
    {
        ERROR_RECOVERABLE("Invalid file type for Loading Obj");
    }
	std::string objFile;
    FileReadToString(objFile, objFilePath);

    std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs;
    std::vector<Face> faces;
    Strings fileLines = SplitStringOnDelimiter(objFile, "\r\n", true);
    positions.reserve(fileLines.size());
	normals.reserve(fileLines.size());
	uvs.reserve(fileLines.size());
	faces.reserve(fileLines.size());
    outVertexes.reserve(fileLines.size());
    outIndexes.reserve(fileLines.size());

    MtlLib* mtlLib = nullptr;
    std::string currMtlkey;
    if (fileLines.size() <= 1)
    {
        fileLines = SplitStringOnDelimiter(objFile, "\n", true);
    }
    if (fileLines.size() <= 0)
    {
		return false;
    }
    for (int lineIdx = 0; lineIdx < fileLines.size(); lineIdx++)
    {
        if (fileLines[lineIdx].substr(0, 7) == "mtllib ")
        {
            std::string mtlLibName = SplitStringOnDelimiter(fileLines[lineIdx], ' ', true)[1];
            Strings fileNameSplit = SplitStringOnDelimiter(fileName, '/', true);
            fileNameSplit[(int)fileNameSplit.size() - 1] = mtlLibName;
            std::string filePath;
            for (int sIdx = 0; sIdx < (int)fileNameSplit.size(); sIdx++)
            {
                filePath += fileNameSplit[sIdx];
                if (sIdx != (int)fileNameSplit.size() - 1)
                {
                    filePath += '/';
                }
            }
            mtlLib = renderer->CreateOrGetMtlLibFromFile(filePath.c_str());
        }
        else if (fileLines[lineIdx].substr(0, 7) == "usemtl ")
        {
            currMtlkey = SplitStringOnDelimiter(fileLines[lineIdx], ' ', true)[1];
            TrimString(currMtlkey, '\n');
        }
        else if (fileLines[lineIdx].substr(0, 2) == "v ")
        {
            Vec3 vertexPos;
            vertexPos.SetFromText(fileLines[lineIdx].substr(2).c_str(), ' ');
            positions.push_back(vertexPos);
        }
        else if (fileLines[lineIdx].substr(0, 2) == "f ")
        {
            if (mtlLib != nullptr)
            {
				Face face(fileLines[lineIdx].substr(2), mtlLib->m_mtls[currMtlkey]);
				faces.push_back(face);
            }
            else
            {
				Face face(fileLines[lineIdx].substr(2), Mtl());
				faces.push_back(face);
            }
        }
        else if (fileLines[lineIdx].substr(0, 3) == "vt ")
        {
            outHasUVs = true;
			Vec2 uv;
            Strings uvAsStrings = SplitStringOnDelimiter(fileLines[lineIdx].substr(3), ' ', true);
            uv.x = (float)atof(uvAsStrings[0].c_str());
			uv.y = (float)atof(uvAsStrings[1].c_str());
            uvs.push_back(uv);
        }
		else if (fileLines[lineIdx].substr(0, 3) == "vn ")
		{
            outHasNormals = true;
			Vec3 normal;
            normal.SetFromText(fileLines[lineIdx].substr(3).c_str(), ' ');
            normals.push_back(normal);
		}
    }

    std::unordered_map<IntVec3, int, VertexHash, VertexEqual> vertHashMap;
    int indexOffset = 0;
    for (int faceIdx = 0; faceIdx < (int)faces.size(); faceIdx++)
    {
        faces[faceIdx].AddFace(outVertexes, outIndexes, positions, uvs, normals, vertHashMap);
		indexOffset += (int)faces[faceIdx].m_vertexes.size();
    }

	CalculateTangentAndBasisVectors(outVertexes, outIndexes, !outHasNormals);
	TransformVertexArray3D(outVertexes, modelTransform, true);

    return true;
}

void ObjLoader::CalculateTangentAndBasisVectors(std::vector<Vertex_PCUTBN>& outVertexes, std::vector<unsigned int>& indexes, bool computeNormals, bool computeTangents)
{
    for (int i = 0; i < (int)indexes.size() - 2; i += 3)
    {
        Vertex_PCUTBN& vert0 = outVertexes[indexes[i]];
		Vertex_PCUTBN& vert1 = outVertexes[indexes[i + 1]];
		Vertex_PCUTBN& vert2 = outVertexes[indexes[i + 2]];

        ComputeTBNForVertex(vert0, vert1, vert2, computeNormals, computeTangents);
    }

    for (int i = 0; i < outVertexes.size(); i++)
    {
		outVertexes[i].m_tangent = outVertexes[i].m_tangent.GetNormalized();
		outVertexes[i].m_bitangent = outVertexes[i].m_bitangent.GetNormalized();
		outVertexes[i].m_normal = outVertexes[i].m_normal.GetNormalized();
        Mat44 tbnBasis;
        tbnBasis.SetIJK3D(outVertexes[i].m_tangent, outVertexes[i].m_bitangent, outVertexes[i].m_normal);
        tbnBasis.Orthonormalize_IFwd_JLeft_KUp_PreserveK();
		outVertexes[i].m_tangent = tbnBasis.GetIBasis3D();
		outVertexes[i].m_bitangent = tbnBasis.GetJBasis3D();
        outVertexes[i].m_normal = tbnBasis.GetKBasis3D();
    }
    
}

void ObjLoader::ComputeTBNForVertex(Vertex_PCUTBN& vert0, Vertex_PCUTBN& vert1, Vertex_PCUTBN& vert2, bool computeNormals, bool computeTangents)
{
	Vec3 p0 = vert0.m_position;
	Vec3 p1 = vert1.m_position;
	Vec3 p2 = vert2.m_position;

	Vec3 e0 = p1 - p0;
	Vec3 e1 = p2 - p0;

    if (computeNormals)
    {
        Vec3 N = CrossProduct3D(e0, e1);
        N = N.GetNormalized();
        vert0.m_normal += N;
        vert1.m_normal += N;
		vert2.m_normal += N;
    }
    if (!computeTangents)
    {
        return;
    }

	float u0 = vert1.m_uvTexCoords.x - vert0.m_uvTexCoords.x;
	float u1 = vert2.m_uvTexCoords.x - vert0.m_uvTexCoords.x;

	float v0 = vert1.m_uvTexCoords.y - vert0.m_uvTexCoords.y;
	float v1 = vert2.m_uvTexCoords.y - vert0.m_uvTexCoords.y;

    float r = 1.f / (u0 * v1 - u1 * v0);
    Vec3 T = r * (v1 * e0 - v0 * e1);
    T = T.GetNormalized();
    Vec3 B = r * (u0 * e1 - u1 * e0);
    B = B.GetNormalized();
    vert0.m_tangent += T;
    vert0.m_bitangent += B;

	vert1.m_tangent += T;
	vert1.m_bitangent += B;

	vert2.m_tangent += T;
	vert2.m_bitangent += B;
}

std::vector<IntVec3> Face::GetOrderedVertexes()
{
    if (m_vertexes.size() <= 3)
    {
        return m_vertexes;
    }

    std::vector<IntVec3> vertexesToReturn;
    for (int i = 1; i < (int)m_vertexes.size() - 1; i++)
    {
        vertexesToReturn.push_back(m_vertexes[0]);
		vertexesToReturn.push_back(m_vertexes[i]);
		vertexesToReturn.push_back(m_vertexes[i + 1]);
    }

    return vertexesToReturn;
}

void Face::AddFace(std::vector<Vertex_PCUTBN>& out_vertexes, std::vector<unsigned int>& out_indexes, std::vector<Vec3> const& positions, std::vector<Vec2> const& uvs, std::vector<Vec3> const& normals,
    std::unordered_map<IntVec3, int, VertexHash, VertexEqual>& vertHashMap)
{
    for (int i = 0; i < (int)m_vertexes.size(); i++)
    {
        IntVec3 faceVert = m_vertexes[i];
        auto iter = vertHashMap.find(faceVert);
        if (iter != vertHashMap.end())
        {
            continue;
        }

		Vertex_PCUTBN vertexToAdd;
        if (faceVert.x > 0)
        {
			vertexToAdd.m_position = positions[faceVert.x - 1];
        }
        if (faceVert.y > 0)
        {
			vertexToAdd.m_uvTexCoords = uvs[faceVert.y - 1];
        }
        if (faceVert.z > 0)
        {
			vertexToAdd.m_normal = normals[faceVert.z - 1];
        }
        vertexToAdd.m_color = m_mtl.m_diffuse;
        vertHashMap[faceVert] = (int)out_vertexes.size();
		out_vertexes.push_back(vertexToAdd);
    }

	for (int i = 1; i < (int)m_vertexes.size() - 1; i++)
	{
        int v0Idx = 0;
        IntVec3 v0Key = m_vertexes[v0Idx];
		out_indexes.push_back(vertHashMap[v0Key]);


		int v1Idx = i;
		IntVec3 v1Key = m_vertexes[v1Idx];
		out_indexes.push_back(vertHashMap[v1Key]);

		int v2Idx = i + 1;
		IntVec3 v2Key = m_vertexes[v2Idx];
		out_indexes.push_back(vertHashMap[v2Key]);
	}
}


std::vector<unsigned int> Face::GetOrderedIndexes()
{
    std::vector<unsigned int> indexesToReturn;
    for (int i = 1; i < (int)m_vertexes.size() - 1; i++)
    {
		indexesToReturn.push_back(0);
		indexesToReturn.push_back(i);
		indexesToReturn.push_back(i + 1);
    }
    return indexesToReturn;
}

