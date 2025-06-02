#include "Engine/Math/FloatCurve.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/FileUtils.hpp"

std::vector<FloatCurve*> FloatCurve::s_floatCurves;

FloatCurve::FloatCurve(std::vector<FloatCurvePoint> const& curvePoints)
	: m_points(curvePoints)
{
}

void FloatCurve::AddPoint(FloatCurvePoint pointToAdd)
{
	if (m_points.size() == 0)
	{
		m_points.push_back(pointToAdd);
		return;
	}
	if (pointToAdd.m_position.x < m_points[0].m_position.x)
	{
		m_points.insert(m_points.begin(), pointToAdd);
	}
	else if (pointToAdd.m_position.x > m_points[m_points.size() - 1].m_position.x)
	{
		m_points.push_back(pointToAdd);
	}

	for (int i = 0; i < (int)m_points.size() - 1; i++)
	{
		if (pointToAdd.m_position.x > m_points[i].m_position.x && pointToAdd.m_position.x < m_points[i + 1].m_position.x)
		{
			m_points.insert(m_points.begin() + (i + 1), pointToAdd);
		}
	}
}

bool FloatCurve::RemovePoint(Vec2 pointPosition)
{
	for (int i = 0; i < (int)m_points.size(); i++)
	{
		if (m_points[i].m_position == pointPosition)
		{
			m_points.erase(m_points.begin() + i);
			return true;
		}
	}
	return false;
}

float FloatCurve::EvaluateAt(float t) const
{
	if (t > 1.f)
	{
		return m_points[m_points.size() - 1].m_position.y;
	}
	if (t <= 0.f)
	{
		return m_points[0].m_position.y;
	}
	int startIdx = 0;
	int endIdx = 0;
	for (int i = 0; i < (int)m_points.size() - 1; i++)
	{
		if (t > m_points[i].m_position.x && t < m_points[i + 1].m_position.x)
		{
			startIdx = i;
			endIdx = i + 1;
			break;
		}
	}
	FloatCurvePoint const& startPoint = m_points[startIdx];
	FloatCurvePoint const& endPoint = m_points[endIdx];

	if (startPoint.m_outgoingMode == InterpolationMode::LINEAR)
	{
		//Lerp between points
		float lerpTime = GetFractionWithinRange(t, startPoint.m_position.x, endPoint.m_position.x);
		return Lerp(startPoint.m_position.y, endPoint.m_position.y, lerpTime);
	}
	return 0.0f;
}

std::vector<Vec2> FloatCurve::GetPositions() const
{
	std::vector<Vec2> positions;
	for (int i = 0; i < (int)m_points.size(); i++)
	{
		positions.push_back(m_points[i].m_position);
	}
	return positions;
}

void FloatCurve::AddVertsForCurve(std::vector<Vertex_PCU>& verts, float lineThickness, AABB2 curveBox, Rgba8 color, int numSubdivisions)
{
	float currT = 0.f;
	float tStep = 1.f / (float)numSubdivisions;

	for (int i = 0; i < numSubdivisions; i++)
	{
		currT = (float)i / (float)numSubdivisions;
		float nextT = currT + tStep;
		Vec2 startSegmentPos = curveBox.GetPointAtUV(Vec2(currT, EvaluateAt(currT)));
		Vec2 endSegmentPos = curveBox.GetPointAtUV(Vec2(nextT, EvaluateAt(nextT)));

		AddVertsForLine2D(verts, startSegmentPos, endSegmentPos, lineThickness, color);
	}
}

bool FloatCurve::SetPosition(Vec2 oldPos, Vec2 newPos)
{
	for (int i = 0; i < (int)m_points.size(); i++)
	{
		if (m_points[i].m_position == oldPos)
		{
			m_points[i].m_position = newPos;
			return true;
		}
	}

	return false;
}

bool FloatCurve::SetPosition(int pointIdx, Vec2 newPos)
{
	if (pointIdx < 0 || pointIdx >= (int)m_points.size())
	{
		return false;
	}
	m_points[pointIdx].m_position = newPos;
	return true;
}

void FloatCurve::ClearCurves()
{
	for (int i = 0; i < (int)s_floatCurves.size(); i++)
	{
		delete s_floatCurves[i];
		s_floatCurves[i] = nullptr;
	}
}

FloatCurve* FloatCurve::GetFloatCurveFromName(std::string name)
{
	for (int i = 0; i < (int)s_floatCurves.size(); i++)
	{
		if (s_floatCurves[i]->m_name == name)
		{
			return s_floatCurves[i];
		}
	}
	return nullptr;
}

void FloatCurve::LoadCurvesFromXML(const char* path)
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile(path) == 0, Stringf("Failed to load curves file %s", path));
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("FloatCurves");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root GameConfig element for curves from %s", path));
	for (XmlElement const* currElement = rootElement->FirstChildElement(); currElement != nullptr; currElement = currElement->NextSiblingElement())
	{
		FloatCurve* floatCurve = new FloatCurve();
		floatCurve->LoadFromXmlElement(*currElement);
		s_floatCurves.push_back(floatCurve);
	}
}

bool FloatCurve::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", "");
	if (m_name == "")
	{
		return false;
	}
	for (XmlElement const* currElement = element.FirstChildElement("Point"); currElement != nullptr; currElement = currElement->NextSiblingElement("Point"))
	{
		FloatCurvePoint point;
		point.m_position = ParseXmlAttribute(*currElement, "position", Vec2::ZERO);
		AddPoint(point);
	}
	return true;
}

void FloatCurve::ClearPoints()
{
	m_points.clear();
}

void FloatCurve::Start0End1()
{
	float startY = m_points[0].m_position.y;
	float endY = m_points[m_points.size() - 1].m_position.y;
	for (int i = 0; i < (int)m_points.size(); i++)
	{
		m_points[i].m_position.y = RangeMap(m_points[i].m_position.y, startY, endY, 0.f, 1.f);
	}
}

void FloatCurve::WriteCurveToXML(std::string name, std::string filePath)
{
	m_name = name;
	std::string floatCurveXML = "";
	floatCurveXML += "<FloatCurve name=\"" + m_name + "\">\n";
	for (int i = 0; i < (int)m_points.size(); i++)
	{
		floatCurveXML += Stringf("<Point position=\"%.3f,%.3f\"/>\n", m_points[i].m_position.x, m_points[i].m_position.y);
	}
	floatCurveXML += "</FloatCurve>";
	WriteStringToFile(floatCurveXML, filePath);
}
