#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"

enum class InterpolationMode
{
	AUTOMATIC = 0,
	LINEAR,
	MANUAL,
	COUNT
};

struct FloatCurvePoint
{
	Vec2 m_position = Vec2::ZERO;
	Vec2 m_incomingVelocity = Vec2::ZERO;
	Vec2 m_outgoingVelocity = Vec2::ZERO;
	InterpolationMode m_incomingMode = InterpolationMode::LINEAR;
	InterpolationMode m_outgoingMode = InterpolationMode::LINEAR;
};

class FloatCurve
{
public:
	FloatCurve() = default;
	FloatCurve(std::vector<FloatCurvePoint> const& curvePoints);
	void AddPoint(FloatCurvePoint pointToAdd);
	bool RemovePoint(Vec2 pointPosition);
	float EvaluateAt(float t) const;
	std::vector<Vec2> GetPositions() const;
	void AddVertsForCurve(std::vector<Vertex_PCU>& verts, float lineThickness, AABB2 curveBox, Rgba8 color = Rgba8::WHITE, int numSubdivisions = 64);
	void WriteCurveToXML(std::string name, std::string filePath);
	bool SetPosition(Vec2 oldPos, Vec2 newPos);
	bool SetPosition(int pointIdx, Vec2 newPos);
	static FloatCurve* GetFloatCurveFromName(std::string name);
	static void ClearCurves();
	static void LoadCurvesFromXML(const char* path);
	bool LoadFromXmlElement(XmlElement const& element);
	void ClearPoints();
	void Start0End1();
	std::string m_name;
private:	
	static std::vector<FloatCurve*> s_floatCurves;
	std::vector<FloatCurvePoint> m_points;

};