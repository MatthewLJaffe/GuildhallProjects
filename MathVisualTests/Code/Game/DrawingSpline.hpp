#pragma once
#include "Game/VisualTest.hpp"
#include "Engine/Math/FloatCurve.hpp"

extern FloatCurve floatCurve;

class DrawingSpline : public VisualTest
{
public:
	DrawingSpline(VisualTestType myTestType, Game* game);
	void Update(float deltaSeconds) override;
	void Render();
	void InitializeTest() override;
	void RandomizeTest() override;
	void HandleInput();
	int m_currEasingFunctionIdx = 0;
	std::vector<Vertex_PCU> m_textVerts;
	std::vector<Vertex_PCU> paneVerts;

	//panes
	AABB2 m_totalViewport;

	std::vector<Vertex_PCU> m_easingFunctionBoxVerts;
	int m_curveSubdivisoins = 64;
	float m_currFloatCurveT = 0.f;
	float m_maxT = 2.f;
	Vec2 m_floatCurvePos;
	int m_selectedPosIdx = -1;
	float m_pointRadius = .75f;
	bool m_showIncomingVelocities = false;
	bool m_showOutgoingvelocities = false;

	Vec2 m_demoCurrPos;
	Vec2 m_demoStartPos;
	Vec2 m_demoEndPos;

	Vec2 m_demoCurrScale;
	Vec2 m_demoStartScale;
	Vec2 m_demoEndScale;

	Rgba8 m_demoCurrColor;
	Rgba8 m_demoStartColor;
	Rgba8 m_demoEndColor;

	float m_demoCurrRotation = 0.f;
	float m_demoStartRotation = 0.f;
	float m_demoEndRotation = 0.f;

private:
	void UpdateFloatCurve();
	void DrawFloatCurve();
};

bool Event_SaveCurveToFile(EventArgs& args);
bool Event_LoadCurveFromFile(EventArgs& args);
bool Event_Start0End1(EventArgs& args);
bool Event_SetPointPos(EventArgs& args);