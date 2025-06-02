#pragma once
#include "Game/VisualTest.hpp"

enum class EasingFunctionType
{
	SMOOTH_START_2,
	SMOOTH_START_3,
	SMOOTH_START_4,
	SMOOTH_START_5,
	SMOOTH_START_6,

	SMOOTH_STOP_2,
	SMOOTH_STOP_3,
	SMOOTH_STOP_4,
	SMOOTH_STOP_5,
	SMOOTH_STOP_6,

	SMOOTH_STEP_3,
	SMOOTH_STEP_5,

	HESITATE_3,
	HESITATE_5,

	CUSTOM_EASING_FUNCTION,

	COUNT
};

struct EasingFunctionData
{
	EasingFunctionData(std::string name, EasingFunctionPtr easingFunctionPtr);
	std::string m_name;
	EasingFunctionPtr m_easingFunctionPtr;
};

class EasingCurvesAndSplinesTest : public VisualTest
{
public:
	EasingCurvesAndSplinesTest(VisualTestType myTestType, Game* game);
	void Update(float deltaSeconds) override;
	void Render();
	void InitializeTest() override;
	void RandomizeTest() override;
	std::vector<EasingFunctionData> m_easingFuncitons;
	int m_currEasingFunctionIdx = 0;
	std::vector<Vertex_PCU> m_textVerts;
	std::vector<Vertex_PCU> paneVerts;

	//panes
	AABB2 m_totalViewport;
	AABB2 m_splinePane;
	AABB2 m_easingFunctionPane;
	AABB2 m_bezierCurvePane;

	//easing functions
	AABB2 m_easingFunctionBox;
	AABB2 m_easingFunctionNameBox;
	Vec2 m_easingFunctionCurrPoint;
	Vec2 m_bezierCurveCurrPoint;
	Vec2 m_bezierCurveGreenPoint;
	std::vector<Vertex_PCU> m_easingFunctionBoxVerts;
	int m_curveSubdivisoins = 64;
	float m_currEasingFuncitonT = 0.f;
	float m_currT = 0.f;
	float m_currCatmullT = 0.f;
	float m_maxT = 2.f;
	CubicBezierCurve2D m_cubicBezier;
	CatmullRomSpline m_catmullRomSpline;
	Vec2 m_catmullPos;
	Vec2 m_catmullGreenPos;
	EasingFunctionType m_currEasingFunction = EasingFunctionType::SMOOTH_START_2;
private:
	void UpdateEasingFunction();
	void UpdateBezierCurve();
	void UpdateCatmullRomSpline();
	void CheckForInputs();
	void DrawCubicBezier();
	void DrawCatmullRomSpline();
};

float CustomFunkyEasingFunction(float t);
float Identity(float t);