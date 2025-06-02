#include "Engine/ParticleSystem/ParticleEmitterDefinition.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

std::string ParticleEmitterDefinition::GetAsXML()
{
	std::string xml;
	EmitterUpdateDefinitionGPU* updateDef = nullptr;
	EmitterRenderDefinitionGPU* renderDef = nullptr;
	
	updateDef = &g_theParticleSystem->m_updateDefinitions[m_loadedDefinitionIndex];
	renderDef = &g_theParticleSystem->m_renderDefinitions[m_loadedDefinitionIndex];
	xml += Stringf("<Emitter name=\"%s\">\n", m_name.c_str());

	xml += Stringf("\t<Emission emissionRate=\"%.1f\" emissionRadius=\"%.2f\" boxDimensions=\"%.3f,%.3f,%.3f\" lifetime=\"%.2f, %.2f\" emissionType=\"%d\" meshFilePath=\"%s\" meshScale=\"%.3f,%.3f,%.3f\" ",
		updateDef->m_emissionRate, updateDef->m_emissionRadius, updateDef->m_boxDimensions[0], updateDef->m_boxDimensions[1], updateDef->m_boxDimensions[2],
		updateDef->m_lifetime[0], updateDef->m_lifetime[1], updateDef->m_emissionType, m_meshFilePath.c_str(), updateDef->m_meshScale[0], updateDef->m_meshScale[1], updateDef->m_meshScale[2]);
	xml += Stringf("emissionMode=\"%d\" repeat=\"%d\" repeatTime=\"%.2f\" emitTime=\"%.2f\" numBursts=\"%d\" burstInterval=\"%.3f\"/>\n",
		updateDef->m_emissionMode, updateDef->m_repeat, updateDef->m_repeatTime, updateDef->m_emitTime, m_numBursts, m_burstInterval);

	xml += Stringf("\t<Physics ignoreWorldPhysics=\"%d\" depthBufferCollisions=\"%d\" velocityXRange=\"%.2f,%.2f\" velocityYRange=\"%.2f,%.2f\" velocityZRange=\"%.2f,%.2f\"",
		updateDef->m_ignoreWorldPhysics, updateDef->m_depthBufferCollisions, updateDef->m_velocityXRange[0], updateDef->m_velocityXRange[1], updateDef->m_velocityYRange[0], updateDef->m_velocityYRange[1], updateDef->m_velocityZRange[0], updateDef->m_velocityZRange[1]);
	xml += Stringf(" inheritEmitterVelocity=\"%d\" inheritVelocityPercentage=\"%.2f\"",
		updateDef->m_inheritEmitterVelocity, updateDef->m_inheritVelocityPercentage);
	xml += Stringf(" setLifetimeVelocity=\"%d\" velocityMode=\"%d\"",
		updateDef->m_setLifetimeVelocity, updateDef->m_velocityMode);
	xml += Stringf(" orientToVelocity=\"%d\" perlinNoiseForce=\"%.2f\"",
		updateDef->m_orientToVelocity, updateDef->m_perlinNoiseForce);
	xml += Stringf("\n\tcurlNoiseAffectPosition=\"%d\" curlNoiseScale=\"%.2f\" curlNoiseSampleSize=\"%.2f\" curlNoiseOctives=\"%d\" curlNoisePan=\"%.2f, %.2f, %.2f\"",
		updateDef->m_curlNoiseAffectPosition, updateDef->m_curlNoiseScale, updateDef->m_curlNoiseSampleSize, updateDef->m_curlNoiseOctives,
		updateDef->m_curlNoisePan[0], updateDef->m_curlNoisePan[1], updateDef->m_curlNoisePan[2]);
	if (updateDef->m_pointForceAttract)
	{
		xml += Stringf("\n\tpointForcePosition=\"%.2f,%.2f,%.2f\" pointForceFalloffExponent=\"%.2f\" pointForceAttract=\"true\" pointForceRadius=\"%.2f\"",
			updateDef->m_pointForcePosition[0], updateDef->m_pointForcePosition[1], updateDef->m_pointForcePosition[2],
			updateDef->m_pointForceFalloffExponent, updateDef->m_pointForceRadius);
	}
	else
	{
		xml += Stringf("\n\tpointForcePosition=\"%.2f,%.2f,%.2f\" pointForceFalloffExponent=\"%.2f\" pointForceAttract=\"false\" pointForceRadius=\"%.2f\"",
			updateDef->m_pointForcePosition[0], updateDef->m_pointForcePosition[1], updateDef->m_pointForcePosition[2],
			updateDef->m_pointForceFalloffExponent, updateDef->m_pointForceRadius);
	}
	xml += Stringf("\n\treturnToOriginForce=\"%.2f\" returnToOriginDelay=\"%.2f\" ignoreForcesWhenAtOrigin=\"%d\"",
		updateDef->m_returnToOriginForce, updateDef->m_returnToOriginDelay, updateDef->m_ignoreForcesWhenAtOrigin);
	xml += Stringf("\n\tvortexAxisDir=\"%.2f,%.2f,%.2f\" vortexAxisOrigin=\"%.2f,%.2f,%.2f\" vortexForceRadius=\"%.2f\">\n",
		updateDef->m_vortexAxisDir[0], updateDef->m_vortexAxisDir[1], updateDef->m_vortexAxisDir[2],
		updateDef->m_vortexAxisOrigin.x, updateDef->m_vortexAxisOrigin.y, updateDef->m_vortexAxisOrigin.z, updateDef->m_vortexForceRadius);
	xml += Stringf("\t</Physics>\n");

	xml += Stringf("\t<Appearance spriteSheetFilePath=\"%s\" spriteSheetDimensions=\"%d,%d\" spriteStartIndex=\"%d\" spriteEndIndex=\"%d\" stretchBillboard=\"%d\" lengthPerSpeed=\"%.2f\" spriteEasingFunction=\"%d\" stretchMode=\"%d\" renderMode=\"%d\" particleMeshFilePath=\"%s\" partialMeshTriangles=\"%d\" softParticles=\"%d\">\n",
		m_spriteSheetFilePath.c_str(), updateDef->m_spriteSheetdimensions[0], updateDef->m_spriteSheetdimensions[1], updateDef->m_spriteStartIndex, updateDef->m_spriteEndIndex, updateDef->m_stretchBillboard, updateDef->m_lengthPerSpeed, updateDef->m_spriteEasingFunction, updateDef->m_stretchMode, m_renderMode, m_particleMeshFilePath.c_str(), updateDef->m_partialMeshTriangles, renderDef->m_softParticles);
	for (unsigned int i = 0; i < updateDef->m_numColorsOverLifetime; i++)
	{
		Rgba8 color;
		color.SetFromFloats(updateDef->m_colorOverLifetime[i].m_color);
		xml += Stringf("\t\t<ColorOverLife color=\"%d,%d,%d,%d\" time=\"%.2f\"/>\n",
			color.r, color.g, color.b, color.a, updateDef->m_colorOverLifetime[i].m_time);
	}
	xml += Stringf("\t\t<PanTexture textureToPan=\"%s\" panTextureSampleScale=\"%.2f,%.2f\" panTextureSpeed=\"%.2f,%.2f\"/>\n",
		m_panTextureFilePath.c_str(), renderDef->m_panTextureSampleScale[0], renderDef->m_panTextureSampleScale[1],
		renderDef->m_panTextureSpeed[0], renderDef->m_panTextureSpeed[1]);
	xml += Stringf("\t</Appearance>\n");

	xml += Stringf("<FloatGraphs>\n");
	for (int i = 0; i < (int)FloatGraphType::NUM_FLOATGRAPHS; i++)
	{
		xml += GetFloatGraphAsXML((FloatGraphType)i);
	}
	xml += Stringf("</FloatGraphs>\n");

	xml += Stringf("<Float2Graphs>\n");
	for (int i = 0; i < (int)Float2GraphType::NUM_FLOAT2GRAPHS; i++)
	{
		xml += GetFloat2GraphAsXML((Float2GraphType)i);
	}
	xml += Stringf("</Float2Graphs>\n");

	xml += Stringf("<Float3Graphs>\n");
	for (int i = 0; i < (int)Float3GraphType::NUM_FLOAT3GRAPHS; i++)
	{
		xml += GetFloat3GraphAsXML((Float3GraphType)i);
	}
	xml += Stringf("</Float3Graphs>\n");

	xml += Stringf("<EmitterProperties lifetime=\"%.2f\" startDelay=\"%.2f\" worldSimulation=\"%d\" emitEmitters=\"%d\" subEmitterFilePath=\"%s\" minSubEmitterOrientation=\"%.2f,%.2f,%.2f\" maxSubEmitterOrientation=\"%.2f,%.2f,%.2f\">\n", 
		m_emitterLifetime, m_emitterStartDelay, updateDef->m_worldSimulation, m_emitEmitters, m_subEmitterFilePath.c_str(),
		m_minSubEmitterOrientation[0], m_minSubEmitterOrientation[1], m_minSubEmitterOrientation[2], m_maxSubEmitterOrientation[0], m_maxSubEmitterOrientation[1], m_maxSubEmitterOrientation[2]);
	xml += GetEmitterVelocityAsXML();
	xml += GetChildEmittersAsXML();
	xml += "</EmitterProperties>\n";
	
	xml += Stringf("</Emitter>\n");
	return xml;
}

std::string ParticleEmitterDefinition::GetFloatGraphAsXML(FloatGraphType floatGraphType)
{
	FloatGraph floatGraph = g_theParticleSystem->m_floatGraphs[m_loadedDefinitionIndex * (int)FloatGraphType::NUM_FLOATGRAPHS + (int)floatGraphType];
	std::string xml;
	xml += Stringf("\t<FloatGraph floatGraphType=\"%d\" constantValue=\"%.3f\" dataMode=\"%d\" minValue=\"%.3f\" maxValue=\"%.3f\">\n", 
		(int)floatGraphType, floatGraph.constantValue, floatGraph.dataMode, floatGraph.minValue, floatGraph.maxValue);
	for (int i = 0; i < (int)floatGraph.numPoints; i++)
	{
		xml += Stringf("\t\t<FloatPoint minValue=\"%.3f\" maxValue=\"%.3f\" time=\"%.3f\" easingFunction=\"%d\"/>\n", floatGraph.points[i].m_minValue, floatGraph.points[i].m_maxValue, floatGraph.points[i].m_time, floatGraph.points[i].m_easingFunction);
	}
	xml += "\t</FloatGraph>\n";
	return xml;
}

std::string ParticleEmitterDefinition::GetFloat2GraphAsXML(Float2GraphType float2GraphType)
{
	Float2Graph float2Graph = g_theParticleSystem->m_float2Graphs[m_loadedDefinitionIndex * (int)Float2GraphType::NUM_FLOAT2GRAPHS + (int)float2GraphType];
	std::string xml;
	xml += Stringf("\t<Float2Graph float2GraphType=\"%d\" constantValue=\"%.3f, %.3f\" dataMode=\"%d\" minValue=\"%.3f, %.3f\" maxValue=\"%.3f, %.3f\">\n",
		(int)float2GraphType, float2Graph.constantValue[0], float2Graph.constantValue[1], float2Graph.dataMode, 
		float2Graph.minValue[0], float2Graph.minValue[1], float2Graph.maxValue[0], float2Graph.maxValue[1]);
	for (int i = 0; i < (int)float2Graph.numPoints; i++)
	{
		xml += Stringf("\t\t<Float2Point minValue=\"%.3f,%.3f\" maxValue=\"%.3f,%.3f\" time=\"%.3f\" easingFunction=\"%d\"/>\n", 
			float2Graph.points[i].m_minValue[0], float2Graph.points[i].m_minValue[1], float2Graph.points[i].m_maxValue[0], float2Graph.points[i].m_maxValue[1], 
			float2Graph.points[i].m_time, float2Graph.points[i].m_easingFunction);
	}
	xml += "\t</Float2Graph>\n";
	return xml;
}

std::string ParticleEmitterDefinition::GetFloat3GraphAsXML(Float3GraphType float3GraphType)
{
	Float3Graph float3Graph = g_theParticleSystem->m_float3Graphs[m_loadedDefinitionIndex * (int)Float3GraphType::NUM_FLOAT3GRAPHS + (int)float3GraphType];
	std::string xml;
	xml += Stringf("\t<Float3Graph float3GraphType=\"%d\" constantValue=\"%.3f,%.3f,%.3f\" dataMode=\"%d\" minValue=\"%.3f,%.3f,%.3f\" maxValue=\"%.3f,%.3f,%.3f\">\n",
		(int)float3GraphType, float3Graph.constantValue[0], float3Graph.constantValue[1], float3Graph.constantValue[2], float3Graph.dataMode,
		float3Graph.minValue[0], float3Graph.minValue[1], float3Graph.minValue[2], float3Graph.maxValue[0], float3Graph.maxValue[1], float3Graph.maxValue[2]);
	for (int i = 0; i < (int)float3Graph.numPoints; i++)
	{
		xml += Stringf("\t\t<Float3Point minValue=\"%.3f,%.3f,%.3f\" maxValue=\"%.3f,%.3f,%.3f\" time=\"%.3f\" easingFunction=\"%d\"/>\n",
			float3Graph.points[i].m_minValue[0], float3Graph.points[i].m_minValue[1], float3Graph.points[i].m_minValue[2], 
			float3Graph.points[i].m_maxValue[0], float3Graph.points[i].m_maxValue[1], float3Graph.points[i].m_maxValue[2],
			float3Graph.points[i].m_time, float3Graph.points[i].m_easingFunction);
	}
	xml += "\t</Float3Graph>\n";
	return xml;
}

std::string ParticleEmitterDefinition::GetChildEmittersAsXML()
{
	std::string xml;
	xml += "\t<EmitterChildren>\n";
	for (int i = 0; i < (int)m_childEmitters.size(); i++)
	{
		xml += Stringf("\t\t<EmitterChild fileName=\"%s\" position=\"%.3f,%.3f,%.3f\" orientation=\"%.3f,%.3f,%.3f\"/>\n", 
			m_childEmitters[i].m_childEmitterFile.c_str(), m_childEmitters[i].m_localPosition.x, m_childEmitters[i].m_localPosition.y, m_childEmitters[i].m_localPosition.z,
			m_childEmitters[i].m_localOrientation.m_yaw, m_childEmitters[i].m_localOrientation.m_pitch, m_childEmitters[i].m_localOrientation.m_roll);
	}
	xml += "\t</EmitterChildren>\n";
	return xml;
}

std::string ParticleEmitterDefinition::GetEmitterVelocityAsXML()
{
	Float3Graph const& float3Graph = g_theParticleSystem->m_emitterVelocityGraphs[m_loadedDefinitionIndex];
	std::string xml;
	xml += Stringf("\t<EmitterVelocity constantValue=\"%.3f,%.3f,%.3f\" dataMode=\"%d\" minValue=\"%.3f,%.3f,%.3f\" maxValue=\"%.3f,%.3f,%.3f\">\n",
		float3Graph.constantValue[0], float3Graph.constantValue[1], float3Graph.constantValue[2], float3Graph.dataMode,
		float3Graph.minValue[0], float3Graph.minValue[1], float3Graph.minValue[2], float3Graph.maxValue[0], float3Graph.maxValue[1], float3Graph.maxValue[2]);
	for (int i = 0; i < (int)float3Graph.numPoints; i++)
	{
		xml += Stringf("\t\t<Float3Point minValue=\"%.3f,%.3f,%.3f\" maxValue=\"%.3f,%.3f,%.3f\" time=\"%.3f\" easingFunction=\"%d\"/>\n",
			float3Graph.points[i].m_minValue[0], float3Graph.points[i].m_minValue[1], float3Graph.points[i].m_minValue[2],
			float3Graph.points[i].m_maxValue[0], float3Graph.points[i].m_maxValue[1], float3Graph.points[i].m_maxValue[2],
			float3Graph.points[i].m_time, float3Graph.points[i].m_easingFunction);
	}
	xml += "\t</EmitterVelocity>\n";
	return xml;
}

void ParticleEmitterDefinition::ParseFloatGraphFromXML(std::vector<FloatGraph>& floatGraphs, XmlElement const& floatGraphElement)
{
	int floatGraphType = ParseXmlAttribute(floatGraphElement, "floatGraphType", 0);
	FloatGraph* floatGraph = &floatGraphs[floatGraphType];
	floatGraph->constantValue = ParseXmlAttribute(floatGraphElement, "constantValue", 0.f);
	floatGraph->dataMode = ParseXmlAttribute(floatGraphElement, "dataMode", 0);
	floatGraph->minValue = ParseXmlAttribute(floatGraphElement, "minValue", 0.f);
	floatGraph->maxValue = ParseXmlAttribute(floatGraphElement, "maxValue", 0.f);
	floatGraph->numPoints = 0;
	for (XmlElement const* floatPointElement = floatGraphElement.FirstChildElement("FloatPoint");
		floatPointElement != nullptr; floatPointElement = floatPointElement->NextSiblingElement("FloatPoint"))
	{
		floatGraph->points[floatGraph->numPoints].m_minValue = ParseXmlAttribute(*floatPointElement, "minValue", 0.f);
		floatGraph->points[floatGraph->numPoints].m_maxValue = ParseXmlAttribute(*floatPointElement, "maxValue", 0.f);
		floatGraph->points[floatGraph->numPoints].m_time = ParseXmlAttribute(*floatPointElement, "time", 0.f);
		floatGraph->points[floatGraph->numPoints].m_easingFunction = ParseXmlAttribute(*floatPointElement, "easingFunction", 0);

		floatGraph->numPoints++;
	}
	floatGraph->isDirty = 1;
}

void ParticleEmitterDefinition::ParseFloat2GraphFromXML(std::vector<Float2Graph>& float2Graphs, XmlElement const& float2GraphElement)
{
	int float2GraphType = ParseXmlAttribute(float2GraphElement, "float2GraphType", 0);
	Float2Graph* float2Graph = &float2Graphs[float2GraphType];
	Vec2 constantValue = ParseXmlAttribute(float2GraphElement, "constantValue", Vec2::ZERO);
	float2Graph->constantValue[0] = constantValue.x;
	float2Graph->constantValue[1] = constantValue.y;

	float2Graph->dataMode = ParseXmlAttribute(float2GraphElement, "dataMode", 0);
	Vec2 minValue = ParseXmlAttribute(float2GraphElement, "minValue", Vec2::ZERO);
	float2Graph->minValue[0] = minValue.x;
	float2Graph->minValue[1] = minValue.y;

	Vec2 maxValue = ParseXmlAttribute(float2GraphElement, "maxValue", Vec2::ZERO);
	float2Graph->maxValue[0] = maxValue.x;
	float2Graph->maxValue[1] = maxValue.y;
	float2Graph->numPoints = 0;

	for (XmlElement const* float2PointElement = float2GraphElement.FirstChildElement("Float2Point");
		float2PointElement != nullptr; float2PointElement = float2PointElement->NextSiblingElement("Float2Point"))
	{
		float2Graph->points[float2Graph->numPoints].m_minValue[0] = ParseXmlAttribute(*float2PointElement, "minValue", Vec2::ZERO).x;
		float2Graph->points[float2Graph->numPoints].m_minValue[1] = ParseXmlAttribute(*float2PointElement, "minValue", Vec2::ZERO).y;

		float2Graph->points[float2Graph->numPoints].m_maxValue[0] = ParseXmlAttribute(*float2PointElement, "maxValue", Vec2::ZERO).x;
		float2Graph->points[float2Graph->numPoints].m_maxValue[1] = ParseXmlAttribute(*float2PointElement, "maxValue", Vec2::ZERO).y;

		float2Graph->points[float2Graph->numPoints].m_time = ParseXmlAttribute(*float2PointElement, "time", 0.f);
		float2Graph->points[float2Graph->numPoints].m_easingFunction = ParseXmlAttribute(*float2PointElement, "easingFunction", 0);

		float2Graph->numPoints++;
	}
	float2Graph->isDirty = 1;
}

void ParticleEmitterDefinition::ParseFloat3GraphFromXML(std::vector<Float3Graph>& float3Graphs, XmlElement const& float3GraphElement)
{
	int float3GraphType = ParseXmlAttribute(float3GraphElement, "float3GraphType", 0);
	Float3Graph* float3Graph = &float3Graphs[float3GraphType];
	Vec3 constantValue = ParseXmlAttribute(float3GraphElement, "constantValue", Vec3::ZERO);
	float3Graph->constantValue[0] = constantValue.x;
	float3Graph->constantValue[1] = constantValue.y;
	float3Graph->constantValue[2] = constantValue.z;

	float3Graph->dataMode = ParseXmlAttribute(float3GraphElement, "dataMode", 0);
	Vec3 minValue = ParseXmlAttribute(float3GraphElement, "minValue", Vec3::ZERO);
	float3Graph->minValue[0] = minValue.x;
	float3Graph->minValue[1] = minValue.y;
	float3Graph->minValue[2] = minValue.z;

	Vec3 maxValue = ParseXmlAttribute(float3GraphElement, "maxValue", Vec3::ZERO);
	float3Graph->maxValue[0] = maxValue.x;
	float3Graph->maxValue[1] = maxValue.y;
	float3Graph->maxValue[2] = maxValue.z;
	float3Graph->numPoints = 0;

	for (XmlElement const* float3PointElement = float3GraphElement.FirstChildElement("Float3Point");
		float3PointElement != nullptr; float3PointElement = float3PointElement->NextSiblingElement("Float3Point"))
	{
		float3Graph->points[float3Graph->numPoints].m_minValue[0] = ParseXmlAttribute(*float3PointElement, "minValue", Vec3::ZERO).x;
		float3Graph->points[float3Graph->numPoints].m_minValue[1] = ParseXmlAttribute(*float3PointElement, "minValue", Vec3::ZERO).y;
		float3Graph->points[float3Graph->numPoints].m_minValue[2] = ParseXmlAttribute(*float3PointElement, "minValue", Vec3::ZERO).z;

		float3Graph->points[float3Graph->numPoints].m_maxValue[0] = ParseXmlAttribute(*float3PointElement, "maxValue", Vec3::ZERO).x;
		float3Graph->points[float3Graph->numPoints].m_maxValue[1] = ParseXmlAttribute(*float3PointElement, "maxValue", Vec3::ZERO).y;
		float3Graph->points[float3Graph->numPoints].m_maxValue[2] = ParseXmlAttribute(*float3PointElement, "maxValue", Vec3::ZERO).z;

		float3Graph->points[float3Graph->numPoints].m_time = ParseXmlAttribute(*float3PointElement, "time", 0.f);
		float3Graph->points[float3Graph->numPoints].m_easingFunction = ParseXmlAttribute(*float3PointElement, "easingFunction", 0);

		float3Graph->numPoints++;
	}
	float3Graph->isDirty = 1;
}

void ParticleEmitterDefinition::ParseEmitterVelocityFromXML(XmlElement const& float3GraphElement)
{
	Float3Graph* float3Graph = &g_theParticleSystem->m_emitterVelocityGraphs[m_loadedDefinitionIndex];
	Vec3 constantValue = ParseXmlAttribute(float3GraphElement, "constantValue", Vec3::ZERO);
	float3Graph->constantValue[0] = constantValue.x;
	float3Graph->constantValue[1] = constantValue.y;
	float3Graph->constantValue[2] = constantValue.z;

	float3Graph->dataMode = ParseXmlAttribute(float3GraphElement, "dataMode", 0);
	Vec3 minValue = ParseXmlAttribute(float3GraphElement, "minValue", Vec3::ZERO);
	float3Graph->minValue[0] = minValue.x;
	float3Graph->minValue[1] = minValue.y;
	float3Graph->minValue[2] = minValue.z;

	Vec3 maxValue = ParseXmlAttribute(float3GraphElement, "maxValue", Vec3::ZERO);
	float3Graph->maxValue[0] = maxValue.x;
	float3Graph->maxValue[1] = maxValue.y;
	float3Graph->maxValue[2] = maxValue.z;
	float3Graph->numPoints = 0;

	for (XmlElement const* float3PointElement = float3GraphElement.FirstChildElement("Float3Point");
		float3PointElement != nullptr; float3PointElement = float3PointElement->NextSiblingElement("Float3Point"))
	{
		float3Graph->points[float3Graph->numPoints].m_minValue[0] = ParseXmlAttribute(*float3PointElement, "minValue", Vec3::ZERO).x;
		float3Graph->points[float3Graph->numPoints].m_minValue[1] = ParseXmlAttribute(*float3PointElement, "minValue", Vec3::ZERO).y;
		float3Graph->points[float3Graph->numPoints].m_minValue[2] = ParseXmlAttribute(*float3PointElement, "minValue", Vec3::ZERO).z;

		float3Graph->points[float3Graph->numPoints].m_maxValue[0] = ParseXmlAttribute(*float3PointElement, "maxValue", Vec3::ZERO).x;
		float3Graph->points[float3Graph->numPoints].m_maxValue[1] = ParseXmlAttribute(*float3PointElement, "maxValue", Vec3::ZERO).y;
		float3Graph->points[float3Graph->numPoints].m_maxValue[2] = ParseXmlAttribute(*float3PointElement, "maxValue", Vec3::ZERO).z;

		float3Graph->points[float3Graph->numPoints].m_time = ParseXmlAttribute(*float3PointElement, "time", 0.f);
		float3Graph->points[float3Graph->numPoints].m_easingFunction = ParseXmlAttribute(*float3PointElement, "easingFunction", 0);

		float3Graph->numPoints++;
	}
}

void ParticleEmitterDefinition::ParseChildrenFromXML(XmlElement const& emitterChildren)
{
	for (XmlElement const* child = emitterChildren.FirstChildElement("EmitterChild");
		child != nullptr; child = child->NextSiblingElement("EmitterChild"))
	{
		ChildEmitter childEmitter;
		childEmitter.m_childEmitterFile = ParseXmlAttribute(*child, "fileName", "Missing");
		childEmitter.m_localPosition = ParseXmlAttribute(*child, "position", Vec3::ZERO);
		childEmitter.m_localOrientation = ParseXmlAttribute(*child, "orientation", EulerAngles());
		childEmitter.m_childEmitterIndex = g_theParticleSystem->CreateOrGetEmitterDefinitionFromFile(childEmitter.m_childEmitterFile);
		childEmitter.m_childEmitterName = g_theParticleSystem->m_loadedEmitterDefinitions[childEmitter.m_childEmitterIndex].m_name;
		m_childEmitters.push_back(childEmitter);
	}
}

void ParticleEmitterDefinition::SetFloatGraphDefaultValues(FloatGraphType floatGraphType, float defaultValue)
{
	FloatGraph* floatGraph = &g_theParticleSystem->m_floatGraphs[m_loadedDefinitionIndex * (int)FloatGraphType::NUM_FLOATGRAPHS + (int)floatGraphType];
	floatGraph->constantValue = defaultValue;
	floatGraph->minValue = defaultValue;
	floatGraph->maxValue = defaultValue;
	floatGraph->numPoints = 1;
	floatGraph->points[0].m_time = 0.f;
	floatGraph->points[0].m_minValue = defaultValue;
	floatGraph->points[0].m_maxValue = defaultValue;
}

void ParticleEmitterDefinition::SetFloat2GraphDefaultValues(Float2GraphType float2GraphType, Vec2 defaultValue)
{
	Float2Graph* floatGraph = &g_theParticleSystem->m_float2Graphs[m_loadedDefinitionIndex * (int)Float2GraphType::NUM_FLOAT2GRAPHS + (int)float2GraphType];
	floatGraph->constantValue[0] = defaultValue.x;
	floatGraph->constantValue[1] = defaultValue.y;

	floatGraph->minValue[0] = defaultValue.x;
	floatGraph->minValue[1] = defaultValue.y;

	floatGraph->maxValue[0] = defaultValue.x;
	floatGraph->maxValue[1] = defaultValue.y;

	floatGraph->numPoints = 1;
	floatGraph->points[0].m_time = 0.f;

	floatGraph->points[0].m_minValue[0] = defaultValue.x;
	floatGraph->points[0].m_minValue[1] = defaultValue.y;
	floatGraph->points[0].m_maxValue[0] = defaultValue.x;
	floatGraph->points[0].m_maxValue[1] = defaultValue.y;
}

void ParticleEmitterDefinition::SetFloat3GraphDefaultValues(Float3GraphType float3GraphType, Vec3 defaultValue)
{
	Float3Graph* floatGraph = &g_theParticleSystem->m_float3Graphs[m_loadedDefinitionIndex * (int)Float3GraphType::NUM_FLOAT3GRAPHS + (int)float3GraphType];

	floatGraph->constantValue[0] = defaultValue.x;
	floatGraph->constantValue[1] = defaultValue.y;
	floatGraph->constantValue[2] = defaultValue.z;

	floatGraph->minValue[0] = defaultValue.x;
	floatGraph->minValue[1] = defaultValue.y;
	floatGraph->minValue[2] = defaultValue.z;

	floatGraph->maxValue[0] = defaultValue.x;
	floatGraph->maxValue[1] = defaultValue.y;
	floatGraph->maxValue[2] = defaultValue.z;

	floatGraph->numPoints = 1;
	floatGraph->points[0].m_time = 0.f;

	floatGraph->points[0].m_minValue[0] = defaultValue.x;
	floatGraph->points[0].m_minValue[1] = defaultValue.y;
	floatGraph->points[0].m_minValue[2] = defaultValue.z;


	floatGraph->points[0].m_maxValue[0] = defaultValue.x;
	floatGraph->points[0].m_maxValue[1] = defaultValue.y;
	floatGraph->points[0].m_maxValue[2] = defaultValue.z;
}

void ParticleEmitterDefinition::SetFloatGraphDefaultValues(FloatGraph& floatGraph, float defaultValue)
{
	floatGraph.constantValue = defaultValue;
	floatGraph.minValue = defaultValue;
	floatGraph.maxValue = defaultValue;
	floatGraph.numPoints = 1;
	floatGraph.points[0].m_time = 0.f;
	floatGraph.points[0].m_minValue = defaultValue;
	floatGraph.points[0].m_maxValue = defaultValue;
	floatGraph.isDirty = 1;
}

void ParticleEmitterDefinition::SetFloat2GraphDefaultValues(Float2Graph& floatGraph, Vec2 defaultValue)
{
	floatGraph.constantValue[0] = defaultValue.x;
	floatGraph.constantValue[1] = defaultValue.y;

	floatGraph.minValue[0] = defaultValue.x;
	floatGraph.minValue[1] = defaultValue.y;

	floatGraph.maxValue[0] = defaultValue.x;
	floatGraph.maxValue[1] = defaultValue.y;

	floatGraph.numPoints = 1;
	floatGraph.points[0].m_time = 0.f;

	floatGraph.points[0].m_minValue[0] = defaultValue.x;
	floatGraph.points[0].m_minValue[1] = defaultValue.y;
	floatGraph.points[0].m_maxValue[0] = defaultValue.x;
	floatGraph.points[0].m_maxValue[1] = defaultValue.y;
	floatGraph.isDirty = 1;
}

void ParticleEmitterDefinition::SetFloat3GraphDefaultValues(Float3Graph& floatGraph, Vec3 defaultValue)
{
	floatGraph.constantValue[0] = defaultValue.x;
	floatGraph.constantValue[1] = defaultValue.y;
	floatGraph.constantValue[2] = defaultValue.z;

	floatGraph.minValue[0] = defaultValue.x;
	floatGraph.minValue[1] = defaultValue.y;
	floatGraph.minValue[2] = defaultValue.z;

	floatGraph.maxValue[0] = defaultValue.x;
	floatGraph.maxValue[1] = defaultValue.y;
	floatGraph.maxValue[2] = defaultValue.z;

	floatGraph.numPoints = 1;
	floatGraph.points[0].m_time = 0.f;

	floatGraph.points[0].m_minValue[0] = defaultValue.x;
	floatGraph.points[0].m_minValue[1] = defaultValue.y;
	floatGraph.points[0].m_minValue[2] = defaultValue.z;

	floatGraph.points[0].m_maxValue[0] = defaultValue.x;
	floatGraph.points[0].m_maxValue[1] = defaultValue.y;
	floatGraph.points[0].m_maxValue[2] = defaultValue.z;
	floatGraph.isDirty = 1;
}

int ParticleEmitterDefinition::CreateEmitterDefinitionFromXML(XmlElement const* emitterXML)
{
	//initialize all data in place and push back at the end
	std::string name = ParseXmlAttribute(*emitterXML, "name", "Missing");
	ParticleEmitterDefinition emitterDef;
	EmitterUpdateDefinitionGPU updateDef;
	EmitterRenderDefinitionGPU renderDef;
	emitterDef.m_name = name;

	std::vector<FloatGraph> localfloatGraphs;
	localfloatGraphs.resize((size_t)FloatGraphType::NUM_FLOATGRAPHS);
	for (int i = 0; i < (int)localfloatGraphs.size(); i++)
	{
		localfloatGraphs[i].isDirty = 0;
	}
	emitterDef.InitializeDefaultFloatGraphValues(&localfloatGraphs);

	std::vector<Float2Graph> localfloat2Graphs;
	localfloat2Graphs.resize((size_t)Float2GraphType::NUM_FLOAT2GRAPHS);
	for (int i = 0; i < (int)localfloat2Graphs.size(); i++)
	{
		localfloat2Graphs[i].isDirty = 0;
	}
	emitterDef.InitializeDefaultFloat2GraphValues(&localfloat2Graphs);

	std::vector<Float3Graph> localfloat3Graphs;
	localfloat3Graphs.resize((size_t)Float3GraphType::NUM_FLOAT3GRAPHS);
	for (int i = 0; i < (int)localfloat3Graphs.size(); i++)
	{
		localfloat3Graphs[i].isDirty = 0;
	}
	emitterDef.InitializeDefaultFloat3GraphValues(&localfloat3Graphs);

	//make sure any dependencies are loaded first
	XmlElement const* emitterProperties = emitterXML->FirstChildElement("EmitterProperties");
	if (emitterProperties != nullptr)
	{
		emitterDef.m_emitterLifetime = ParseXmlAttribute(*emitterProperties, "lifetime", -1.f);
		emitterDef.m_emitterStartDelay = ParseXmlAttribute(*emitterProperties, "startDelay", -1.f);
		emitterDef.m_emitEmitters = (bool)ParseXmlAttribute(*emitterProperties, "emitEmitters", 0);
		updateDef.m_worldSimulation = ParseXmlAttribute(*emitterProperties, "worldSimulation", 0);
		emitterDef.m_subEmitterFilePath = ParseXmlAttribute(*emitterProperties, "subEmitterFilePath", "");
		if (emitterDef.m_subEmitterFilePath != "")
		{
			emitterDef.m_subEmitterDefinitionIndex = g_theParticleSystem->CreateOrGetEmitterDefinitionFromFile(emitterDef.m_subEmitterFilePath);
		}

		Vec3 minSubEmitterOrientation = ParseXmlAttribute(*emitterProperties, "minSubEmitterOrientation", Vec3::ZERO);
		minSubEmitterOrientation.GetFromFloats(emitterDef.m_minSubEmitterOrientation);
		Vec3 maxSubEmitterOrientation = ParseXmlAttribute(*emitterProperties, "maxSubEmitterOrientation", Vec3::ZERO);
		maxSubEmitterOrientation.GetFromFloats(emitterDef.m_maxSubEmitterOrientation);
		XmlElement const* childEmitters = emitterProperties->FirstChildElement("EmitterChildren");
		if (childEmitters != nullptr)
		{
			emitterDef.ParseChildrenFromXML(*childEmitters);
		}
	}

	//parse all data
	//emission
	XmlElement const* emission = emitterXML->FirstChildElement("Emission");
	updateDef.m_emissionRate = ParseXmlAttribute(*emission, "emissionRate", 0.f);
	updateDef.m_emissionRadius = ParseXmlAttribute(*emission, "emissionRadius", 0.f);
	std::string lifetimeString = emission->Attribute("lifetime");
	if (SplitStringOnDelimiter(lifetimeString, ',').size() == 2)
	{
		Vec2 lifetimeRange = ParseXmlAttribute(*emission, "lifetime", Vec2(1.f, 1.f));
		updateDef.m_lifetime[0] = lifetimeRange.x;
		updateDef.m_lifetime[1] = lifetimeRange.y;

		renderDef.m_lifetime[0] = lifetimeRange.x;
		renderDef.m_lifetime[1] = lifetimeRange.y;
	}
	else
	{
		float lifetimeValue = ParseXmlAttribute(*emission, "lifetime", 1.f);
		updateDef.m_lifetime[0] = lifetimeValue;
		updateDef.m_lifetime[1] = lifetimeValue;

		renderDef.m_lifetime[0] = lifetimeValue;
		renderDef.m_lifetime[1] = lifetimeValue;
	}
	updateDef.m_emissionType = ParseXmlAttribute(*emission, "emissionType", 0);
	Vec3 meshScale = ParseXmlAttribute(*emission, "meshScale", Vec3(1.f, 1.f, 1.f));
	updateDef.m_meshScale[0] = meshScale.x;
	updateDef.m_meshScale[1] = meshScale.y;
	updateDef.m_meshScale[2] = meshScale.z;

	Vec3 boxDimensions = ParseXmlAttribute(*emission, "boxDimensions", Vec3(1.f, 1.f, 1.f));
	updateDef.m_boxDimensions[0] = boxDimensions.x;
	updateDef.m_boxDimensions[1] = boxDimensions.y;
	updateDef.m_boxDimensions[2] = boxDimensions.z;

	emitterDef.m_meshFilePath = ParseXmlAttribute(*emission, "meshFilePath", "");
	if (emitterDef.m_meshFilePath != "")
	{
		updateDef.m_meshEntryEmissionIndex = g_theParticleSystem->CreateOrGetMeshParticleEntry(emitterDef.m_meshFilePath);
	}

	updateDef.m_emissionMode = ParseXmlAttribute(*emission, "emissionMode", 0);
	updateDef.m_repeat = ParseXmlAttribute(*emission, "repeat", 0);
	updateDef.m_repeatTime = ParseXmlAttribute(*emission, "repeatTime", 1.f);
	updateDef.m_emitTime = ParseXmlAttribute(*emission, "emitTime", -1.f);
	emitterDef.m_numBursts = ParseXmlAttribute(*emission, "numBursts", 1);
	emitterDef.m_burstInterval = ParseXmlAttribute(*emission, "burstInterval", .1f);


	//physics
	XmlElement const* physics = emitterXML->FirstChildElement("Physics");
	updateDef.m_ignoreWorldPhysics = ParseXmlAttribute(*physics, "ignoreWorldPhysics", 1);
	updateDef.m_depthBufferCollisions = (unsigned int)ParseXmlAttribute(*physics, "depthBufferCollisions", 0);
	Vec2 velocityXRange = ParseXmlAttribute(*physics, "velocityXRange", Vec2::ZERO);
	Vec2 velocityYRange = ParseXmlAttribute(*physics, "velocityYRange", Vec2::ZERO);
	Vec2 velocityZRange = ParseXmlAttribute(*physics, "velocityZRange", Vec2::ZERO);

	updateDef.m_inheritEmitterVelocity = (unsigned int)ParseXmlAttribute(*physics, "inheritEmitterVelocity", 0);
	updateDef.m_inheritVelocityPercentage = ParseXmlAttribute(*physics, "inheritVelocityPercentage", 1.f);

	updateDef.m_velocityXRange[0] = velocityXRange.x;
	updateDef.m_velocityXRange[1] = velocityXRange.y;
	updateDef.m_velocityYRange[0] = velocityYRange.x;
	updateDef.m_velocityYRange[1] = velocityYRange.y;
	updateDef.m_velocityZRange[0] = velocityZRange.x;
	updateDef.m_velocityZRange[1] = velocityZRange.y;

	updateDef.m_setLifetimeVelocity = ParseXmlAttribute(*physics, "setLifetimeVelocity", 0);
	updateDef.m_velocityMode = ParseXmlAttribute(*physics, "velocityMode", 0);

	updateDef.m_orientToVelocity = ParseXmlAttribute(*physics, "orientToVelocity", 0);
	float constantMaxSpeed = ParseXmlAttribute(*physics, "maxSpeed", -1.f);
	if (constantMaxSpeed != -1.f)
	{
		SetFloatGraphDefaultValues(localfloatGraphs[(int)FloatGraphType::FLOATGRAPH_MAX_SPEED], constantMaxSpeed);
		//emitterDef.SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_MAX_SPEED, constantMaxSpeed);
	}
	updateDef.m_perlinNoiseForce = ParseXmlAttribute(*physics, "perlinNoiseForce", 0.f);

	Vec3 constantLinearForce = ParseXmlAttribute(*physics, "linearForce", Vec3(-999.f, -999.f, -999.f));
	if (constantLinearForce != Vec3(-999.f, -999.f, -999.f))
	{
		SetFloat3GraphDefaultValues(localfloat3Graphs[(int)Float3GraphType::FLOAT3GRAPH_LINEAR_FORCE], constantLinearForce);
		//emitterDef.SetFloat3GraphDefaultValues(Float3GraphType::FLOAT3GRAPH_LINEAR_FORCE, constantLinearForce);
	}

	float constantCurlNoise = ParseXmlAttribute(*physics, "curlNoiseForce", -1.f);
	if (constantCurlNoise != -1.f)
	{
		SetFloatGraphDefaultValues(localfloatGraphs[(int)FloatGraphType::FLOATGRAPH_CURL_NOISE_FORCE], constantCurlNoise);
		//emitterDef.SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_CURL_NOISE_FORCE, constantCurlNoise);
	}
	updateDef.m_curlNoiseAffectPosition = ParseXmlAttribute(*physics, "curlNoiseAffectPosition", 0);
	updateDef.m_curlNoiseScale = ParseXmlAttribute(*physics, "curlNoiseScale", 0.f);
	updateDef.m_curlNoiseSampleSize = ParseXmlAttribute(*physics, "curlNoiseSampleSize", 0.f);
	updateDef.m_curlNoiseOctives = ParseXmlAttribute(*physics, "curlNoiseOctives", 0);

	Vec3 curlNoisePan = ParseXmlAttribute(*physics, "curlNoisePan", Vec3::ZERO);
	updateDef.m_curlNoisePan[0] = curlNoisePan.x;
	updateDef.m_curlNoisePan[1] = curlNoisePan.y;
	updateDef.m_curlNoisePan[2] = curlNoisePan.z;

	float constantPointForce = ParseXmlAttribute(*physics, "pointForceStrength", -1.f);
	if (constantPointForce != -1.f)
	{
		SetFloatGraphDefaultValues(localfloatGraphs[(int)FloatGraphType::FLOATGRAPH_POINT_FORCE], constantPointForce);
		//emitterDef.SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_POINT_FORCE, constantPointForce);
	}

	Vec3 pointForcePosition = ParseXmlAttribute(*physics, "pointForcePosition", Vec3(0.f, 0.f, 0.f));
	updateDef.m_pointForcePosition[0] = pointForcePosition.x;
	updateDef.m_pointForcePosition[1] = pointForcePosition.y;
	updateDef.m_pointForcePosition[2] = pointForcePosition.z;

	updateDef.m_pointForceFalloffExponent = ParseXmlAttribute(*physics, "pointForceFalloffExponent", 1.f);
	updateDef.m_pointForceAttract = ParseXmlAttribute(*physics, "pointForceAttract", true);
	updateDef.m_pointForceRadius = ParseXmlAttribute(*physics, "pointForceRadius", 0.f);

	Vec3 vortexAxisDir = ParseXmlAttribute(*physics, "vortexAxisDir", Vec3(1.f, 0.f, 0.f));
	updateDef.m_vortexAxisDir[0] = vortexAxisDir.x;
	updateDef.m_vortexAxisDir[1] = vortexAxisDir.y;
	updateDef.m_vortexAxisDir[2] = vortexAxisDir.z;

	float constantVortexForce = ParseXmlAttribute(*physics, "vortexForce", -1.f);
	if (constantVortexForce != -1.f)
	{
		SetFloatGraphDefaultValues(localfloatGraphs[(int)FloatGraphType::FLOATGRAPH_VORTEX_FORCE], constantVortexForce);
		//emitterDef.SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_VORTEX_FORCE, constantVortexForce);
	}

	Vec3 vortexAxisOrigin = ParseXmlAttribute(*physics, "vortexAxisOrigin", Vec3(0.f, 0.f, 0.f));
	updateDef.m_vortexAxisOrigin = vortexAxisOrigin;
	updateDef.m_vortexForceRadius = ParseXmlAttribute(*physics, "vortexForceRadius",0.f);

	float constantDragForce = ParseXmlAttribute(*physics, "dragForce", -1.f);
	if (constantDragForce != -1.f)
	{
		SetFloatGraphDefaultValues(localfloatGraphs[(int)FloatGraphType::FLOATGRAPH_DRAG_FORCE], constantDragForce);
		//emitterDef.SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_DRAG_FORCE, constantDragForce);
	}

	updateDef.m_returnToOriginForce = ParseXmlAttribute(*physics, "returnToOriginForce", 0.f);
	updateDef.m_returnToOriginDelay = ParseXmlAttribute(*physics, "returnToOriginDelay", 1.f);
	updateDef.m_ignoreForcesWhenAtOrigin = ParseXmlAttribute(*physics, "ignoreForcesWhenAtOrigin", 0);

	//appearance
	XmlElement const* appearance = emitterXML->FirstChildElement("Appearance");
	int numColorEntries = 0;
	for (XmlElement const* colorOverLife = appearance->FirstChildElement("ColorOverLife");
		colorOverLife != nullptr; colorOverLife = colorOverLife->NextSiblingElement("ColorOverLife"))
	{
		Rgba8 color = ParseXmlAttribute(*colorOverLife, "color", Rgba8::WHITE);
		color.GetAsFloats(updateDef.m_colorOverLifetime[numColorEntries].m_color);
		updateDef.m_colorOverLifetime[numColorEntries].m_time = ParseXmlAttribute(*colorOverLife, "time", 0.f);

		numColorEntries++;
	}
	updateDef.m_numColorsOverLifetime = numColorEntries;
	updateDef.m_stretchBillboard = ParseXmlAttribute(*appearance, "stretchBillboard", 0);
	updateDef.m_lengthPerSpeed = ParseXmlAttribute(*appearance, "lengthPerSpeed", .1f);
	updateDef.m_stretchMode = ParseXmlAttribute(*appearance, "stretchMode", 0);
	emitterDef.m_renderMode = ParseXmlAttribute(*appearance, "renderMode", 0);
	emitterDef.m_particleMeshFilePath = ParseXmlAttribute(*appearance, "particleMeshFilePath", "");
	//render particles as meshes
	if (emitterDef.m_renderMode == 1)
	{
		updateDef.m_particleMeshIndex = g_theParticleSystem->CreateOrGetMeshParticleEntry(emitterDef.m_particleMeshFilePath);
	}
	if (emitterDef.m_renderMode == 2)
	{
		updateDef.m_particleMeshIndex = g_theParticleSystem->CreateOrGetMeshParticleEntry(emitterDef.m_particleMeshFilePath);
		updateDef.m_partialMeshTriangles = ParseXmlAttribute(*appearance, "partialMeshTriangles", 0);
	}

	//support for old size data
	Float2Graph* sizeGraph = &localfloat2Graphs[(int)Float2GraphType::FLOAT2GRAPH_SIZE];
	//Float2Graph* sizeGraph = &g_theParticleSystem->m_float2Graphs[(int)Float2GraphType::FLOAT2GRAPH_SIZE + (int)Float2GraphType::NUM_FLOAT2GRAPHS * emitterDef.m_loadedDefinitionIndex];
	for (XmlElement const* sizeOverLife = appearance->FirstChildElement("SizeOverLife");
		sizeOverLife != nullptr; sizeOverLife = sizeOverLife->NextSiblingElement("SizeOverLife"))
	{
		Vec2 size = ParseXmlAttribute(*sizeOverLife, "size", Vec2::ONE);
		sizeGraph->points[sizeGraph->numPoints].m_minValue[0] = size.x;
		sizeGraph->points[sizeGraph->numPoints].m_minValue[1] = size.y;

		sizeGraph->points[sizeGraph->numPoints].m_maxValue[0] = size.x;
		sizeGraph->points[sizeGraph->numPoints].m_maxValue[1] = size.y;
		sizeGraph->points[sizeGraph->numPoints].m_easingFunction = ParseXmlAttribute(*sizeOverLife, "easingFunction", 0);
		sizeGraph->points[sizeGraph->numPoints].m_time = ParseXmlAttribute(*sizeOverLife, "time", 0.f);
		sizeGraph->numPoints++;
		sizeGraph->isDirty = 1;
	}
	if (sizeGraph->numPoints == 1)
	{
		sizeGraph->dataMode = 0;
		sizeGraph->constantValue[0] = sizeGraph->points[0].m_minValue[0];
		sizeGraph->constantValue[1] = sizeGraph->points[0].m_minValue[1];
		sizeGraph->minValue[0] = sizeGraph->points[0].m_minValue[0];
		sizeGraph->minValue[1] = sizeGraph->points[0].m_minValue[1];
		sizeGraph->maxValue[0] = sizeGraph->points[0].m_maxValue[0];
		sizeGraph->maxValue[1] = sizeGraph->points[0].m_maxValue[1];
	}
	else if (sizeGraph->numPoints > 1)
	{
		sizeGraph->dataMode = 2;
	}

	emitterDef.m_spriteSheetFilePath = ParseXmlAttribute(*appearance, "spriteSheetFilePath", "Data/Images/sprite_sheet.png");
	AABB2 imageBounds = g_theParticleSystem->GetImageBoundsInSpriteAtlas(emitterDef.m_spriteSheetFilePath);
	updateDef.m_atlasUVMins = imageBounds.m_mins;
	updateDef.m_atlasUVMaxs = imageBounds.m_maxs;

	IntVec2 spriteSheetDimensions = ParseXmlAttribute(*appearance, "spriteSheetDimensions", IntVec2::ZERO);
	updateDef.m_spriteSheetdimensions[0] = spriteSheetDimensions.x;
	updateDef.m_spriteSheetdimensions[1] = spriteSheetDimensions.y;
	updateDef.m_spriteStartIndex = ParseXmlAttribute(*appearance, "spriteStartIndex", 0);
	updateDef.m_spriteEndIndex = ParseXmlAttribute(*appearance, "spriteEndIndex", 0);
	updateDef.m_spriteEasingFunction = ParseXmlAttribute(*appearance, "spriteEasingFunction", 0);
	float constantEmissive = ParseXmlAttribute(*appearance, "emissive", -1.f);
	renderDef.m_softParticles = (unsigned int)ParseXmlAttribute(*appearance, "softParticles", 1);
	
	if (constantEmissive != -1.f)
	{
		SetFloatGraphDefaultValues(localfloatGraphs[(int)FloatGraphType::FLOATGRAPH_EMISSIVE], constantEmissive);
		//emitterDef.SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_EMISSIVE, constantEmissive);
	}

	float constantAlphaObscurance = ParseXmlAttribute(*appearance, "alphaObscurance", -1.f);
	if (constantAlphaObscurance != -1.f)
	{
		SetFloatGraphDefaultValues(localfloatGraphs[(int)FloatGraphType::FLOATGRAPH_ALPHA_OBSCURANCE], constantAlphaObscurance);
		//emitterDef.SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_ALPHA_OBSCURANCE, constantAlphaObscurance);
	}

	XmlElement const* panTexture = appearance->FirstChildElement("PanTexture");
	if (panTexture != nullptr)
	{
		emitterDef.m_panTextureFilePath = ParseXmlAttribute(*panTexture, "textureToPan", "");
		if (emitterDef.m_panTextureFilePath != "")
		{
			renderDef.m_panTextureUVBounds = g_theParticleSystem->GetImageBoundsInSpriteAtlas(emitterDef.m_panTextureFilePath);
		}
		float constantPanTextureContribution = ParseXmlAttribute(*panTexture, "panTextureContribution", 0.f);
		if (constantPanTextureContribution != -1.f)
		{
			SetFloatGraphDefaultValues(localfloatGraphs[(int)FloatGraphType::FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION], constantPanTextureContribution);
			//emitterDef.SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION, constantPanTextureContribution);
		}
		Vec2 panTextureSampleScale = ParseXmlAttribute(*panTexture, "panTextureSampleScale", Vec2::ONE);
		renderDef.m_panTextureSampleScale[0] = panTextureSampleScale.x;
		renderDef.m_panTextureSampleScale[1] = panTextureSampleScale.y;
		Vec2 panTextureSpeed = ParseXmlAttribute(*panTexture, "panTextureSpeed", Vec2::ONE);
		renderDef.m_panTextureSpeed[0] = panTextureSpeed.x;
		renderDef.m_panTextureSpeed[1] = panTextureSpeed.y;
	}

	XmlElement const* floatGraphs = emitterXML->FirstChildElement("FloatGraphs");
	if (floatGraphs != nullptr)
	{
		for (XmlElement const* floatGraph = floatGraphs->FirstChildElement("FloatGraph");
			floatGraph != nullptr; floatGraph = floatGraph->NextSiblingElement("FloatGraph"))
		{
			emitterDef.ParseFloatGraphFromXML(localfloatGraphs, *floatGraph);
		}
	}

	XmlElement const* float2Graphs = emitterXML->FirstChildElement("Float2Graphs");
	if (float2Graphs != nullptr)
	{
		for (XmlElement const* float2Graph = float2Graphs->FirstChildElement("Float2Graph");
			float2Graph != nullptr; float2Graph = float2Graph->NextSiblingElement("Float2Graph"))
		{
			emitterDef.ParseFloat2GraphFromXML(localfloat2Graphs, *float2Graph);
		}
	}

	XmlElement const* float3Graphs = emitterXML->FirstChildElement("Float3Graphs");
	if (float3Graphs != nullptr)
	{
		for (XmlElement const* float3Graph = float3Graphs->FirstChildElement("Float3Graph");
			float3Graph != nullptr; float3Graph = float3Graph->NextSiblingElement("Float3Graph"))
		{
			emitterDef.ParseFloat3GraphFromXML(localfloat3Graphs, *float3Graph);
		}
	}

	//now push back all data
	emitterDef.m_loadedDefinitionIndex = (int)g_theParticleSystem->m_loadedEmitterDefinitions.size();
	g_theParticleSystem->m_loadedEmitterDefinitions.push_back(emitterDef);
	g_theParticleSystem->m_updateDefinitions.push_back(updateDef);
	g_theParticleSystem->m_renderDefinitions.push_back(renderDef);

	for (int i = 0; i < (int)localfloatGraphs.size(); i++)
	{
		g_theParticleSystem->m_floatGraphs.push_back(localfloatGraphs[i]);
	}
	for (int i = 0; i < (int)localfloat2Graphs.size(); i++)
	{
		g_theParticleSystem->m_float2Graphs.push_back(localfloat2Graphs[i]);
	}	
	for (int i = 0; i < (int)localfloat3Graphs.size(); i++)
	{
		g_theParticleSystem->m_float3Graphs.push_back(localfloat3Graphs[i]);
	}

	//emitter velocity needs to be initialized last since it is modifying the data in the system not locally
	g_theParticleSystem->m_emitterVelocityGraphs.emplace_back();
	if (emitterProperties != nullptr)
	{
		XmlElement const* emitterVelocity = emitterProperties->FirstChildElement("EmitterVelocity");
		if (emitterVelocity != nullptr)
		{
			emitterDef.ParseEmitterVelocityFromXML(*emitterVelocity);
		}
	}
	return emitterDef.m_loadedDefinitionIndex;
}

void ParticleEmitterDefinition::InitializeDefaultFloatGraphValues(std::vector<FloatGraph>* localFloatGraphs)
{
	if (localFloatGraphs == nullptr)
	{
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_ALPHA_OBSCURANCE, 1.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_CURL_NOISE_FORCE, 0.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_DRAG_FORCE, 0.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_EMISSIVE, 0.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_MAX_SPEED, 10.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION, 0.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_POINT_FORCE, 0.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_VORTEX_FORCE, 0.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_RADIAL_VELOCITY, 0.f);
		SetFloatGraphDefaultValues(FloatGraphType::FLOATGRAPH_ROTATION_1D, 0.f);
	}
	else
	{
		std::vector<FloatGraph>& floatGraphs = *localFloatGraphs;
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_ALPHA_OBSCURANCE], 1.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_CURL_NOISE_FORCE], 0.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_DRAG_FORCE], 0.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_EMISSIVE], 0.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_MAX_SPEED], 10.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION], 0.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_POINT_FORCE], 0.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_VORTEX_FORCE], 0.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_RADIAL_VELOCITY], 0.f);
		SetFloatGraphDefaultValues(floatGraphs[(int)FloatGraphType::FLOATGRAPH_ROTATION_1D], 0.f);
	}

}

void ParticleEmitterDefinition::InitializeDefaultFloat2GraphValues(std::vector<Float2Graph>* localFloat2Graphs)
{
	if (localFloat2Graphs == nullptr)
	{
		SetFloat2GraphDefaultValues(Float2GraphType::FLOAT2GRAPH_SIZE, Vec2::ONE);
	}
	else
	{
		std::vector<Float2Graph> float2Graphs = *localFloat2Graphs;
		SetFloat2GraphDefaultValues(float2Graphs[(int)Float2GraphType::FLOAT2GRAPH_SIZE], Vec2::ONE);

	}
}

void ParticleEmitterDefinition::InitializeDefaultFloat3GraphValues(std::vector<Float3Graph>* localFloat3Graphs)
{
	if (localFloat3Graphs == nullptr)
	{
		SetFloat3GraphDefaultValues(Float3GraphType::FLOAT3GRAPH_LIFETIME_POSITION, Vec3::ZERO);
		SetFloat3GraphDefaultValues(Float3GraphType::FLOAT3GRAPH_LIFETIME_VELOCITY, Vec3(0.f, 0.f, 10.f));
		SetFloat3GraphDefaultValues(Float3GraphType::FLOAT3GRAPH_LINEAR_FORCE, Vec3(0.f, 0.f, 10.f));
		SetFloat3GraphDefaultValues(Float3GraphType::FLOAT3GRAPH_LIFETIME_ROTATION_3D, Vec3(0.f, 0.f, 0.f));
		SetFloat3GraphDefaultValues(Float3GraphType::FLOAT3GRAPH_LIFETIME_SCALE_3D, Vec3(1.f, 1.f, 1.f));
	}
	else
	{
		std::vector<Float3Graph> float3Graphs = *localFloat3Graphs;
		SetFloat3GraphDefaultValues(float3Graphs[(int)Float3GraphType::FLOAT3GRAPH_LIFETIME_POSITION], Vec3::ZERO);
		SetFloat3GraphDefaultValues(float3Graphs[(int)Float3GraphType::FLOAT3GRAPH_LIFETIME_VELOCITY], Vec3(0.f, 0.f, 10.f));
		SetFloat3GraphDefaultValues(float3Graphs[(int)Float3GraphType::FLOAT3GRAPH_LINEAR_FORCE], Vec3(0.f, 0.f, 10.f));
		SetFloat3GraphDefaultValues(float3Graphs[(int)Float3GraphType::FLOAT3GRAPH_LIFETIME_ROTATION_3D], Vec3(0.f, 0.f, 0.f));
		SetFloat3GraphDefaultValues(float3Graphs[(int)Float3GraphType::FLOAT3GRAPH_LIFETIME_SCALE_3D], Vec3(1.f, 1.f, 1.f));
	}
}