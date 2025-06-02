#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Timer.hpp"

enum class DebugRenderMode
{
	ALWAYS,
	USE_DEPTH,
	X_RAY
};

struct DebugRenderConfig
{
	Renderer* m_renderer = nullptr;
	std::string m_fontName = "SquirrelFixedFont";
};

//Setup
void DebugRenderSystemStartup(const DebugRenderConfig& config);
void DebugRenderSystemShutdown();

//Control
void DebugRenderSetVisisble();
void DebugRenderSetHidden();
void DebugRenderSetHidden();
void DebugRenderToggle();
void DebugRenderClear();
void DebugRemoveScreenText(std::string text);
void DebugRemoveMessage(std::string text);


//Output
void DebugRenderBeginFrame();
void DebugRenderWorld(const Camera& camera);
void DebugRenderScreen(const Camera& camera);
void DebugRenderEndFrame();

//Geometry
void DebugAddWorldPoint(const Vec3& pos, 
	float radius, float duration, 
	const Rgba8& startColor = Rgba8::WHITE, 
	const Rgba8& endColor = Rgba8::WHITE, 
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldLine(const Vec3& start, const Vec3& end,
	float radius, float duration, 
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE, 
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldAABB3(const Vec3& mins, const Vec3& maxs,
	float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldBoxLine(const Vec3& start, const Vec3& end,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top,
	float radius, float duration, 
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE, 
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldWireSphere(Vec3 const& center,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE, 
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldArrow(const Vec3& start, const Vec3& end, 
	float radius, float duration, 
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldText(const std::string& text,
	const Mat44& transform, float textHeight,
	const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldBillboardText(const std::string& text,
	const Vec3& origin, float textHeight,
	const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8 endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddScreenText(const std::string& text,
	const Vec2& position, float size,
	const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE);

void DebugAddMessage(const std::string& text,
	float height, float duration, 
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE);

void DebugAddWorldBasis(const Mat44& transform, float duration, DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

// Console commands
bool Command_DebugRenderClear(EventArgs& args);
bool Command_DebugRenderToggle(EventArgs& args);
