#pragma once
#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

constexpr int MAX_MISSILES = 100;
constexpr int MAX_BULLETS = 1000;
constexpr int MAX_BEETLES = 20;
constexpr int MAX_AMMO_PICKUPS = 50;
constexpr int NUM_SHIP_VERTS = 15;
constexpr int NUM_TAIL_VERTS = 3;
constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
constexpr int NUM_STARTING_ASTEROIDS = 6;
constexpr int MAX_ASTEROIDS = 12;
constexpr float ASTEROID_SPEED = 5.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 3.2f;
constexpr float ASTEROID_COSMETIC_RADIUS = 4.0f;
constexpr int NUM_BULLET_VERTS = 6;
constexpr float BULLET_LIFETIME_SECONDS = 10.0f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;
constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr float BEETLE_SPEED = 6.f;
constexpr float BEETLE_COSMETIC_RADIUS = 2.f;
constexpr float BEETLE_PHYSICS_RADIUS = 1.25f;
constexpr int NUM_BEETLE_VERTS = 6;
constexpr int NUM_WASP_VERTS = 6;
constexpr int NUM_SCOUT_VERTS = 6;
constexpr int NUM_CHARGER_VERTS = 6;
constexpr int NUM_SHOTGUNNER_VERTS = 6;
constexpr int NUM_MISSILE_VERTS = 15;
constexpr float WASP_SPEED = 15.f;
constexpr float WASP_COSMETIC_RADIUS = 1.75f;
constexpr float WASP_PHYSICS_RADIUS = 1.f;
constexpr float WASP_ACCELERATION_FORCE = 10;
constexpr int MAX_WASPS = 20;
constexpr int MAX_DEBRIS = 500;
constexpr int MAX_SCOUTS = 200;
constexpr int MAX_CHARGERS = 200;
constexpr int MAX_SHOTGUNNERS = 200;
constexpr float DEBRIS_SPEED = 25.f;
constexpr int NUM_WAVES = 5;

constexpr float SCOUT_COSMETIC_RADIUS = 2.f;
constexpr float SCOUT_PHYSICS_RADIUS = 1.25f;
constexpr float CHARGER_COSMETIC_RADIUS = 2.f;
constexpr float CHARGER_PHYSICS_RADIUS = 1.25f;
constexpr float SHOTGUNNER_COSMETIC_RADIUS = 2.f;
constexpr float SHOTGUNNER_PHYSICS_RADIUS = 1.25f;
constexpr float MISSILE_PHYSICS_RADIUS = 2.f;
constexpr float MISSILE_COSMETIC_RADIUS = 4.f;

constexpr int PLAYER_SHIP_ID = 1;
constexpr int BULLET_ID = 2;
constexpr int ASTEROID_ID = 3;
constexpr int BEETLE_ID = 4;
constexpr int WASP_ID = 5;
constexpr int DEBRIS_ID = 6;
constexpr int SCOUT_ID = 7;
constexpr int CHARGER_ID = 8;
constexpr int SHOTGUNNER_ID = 9;
constexpr int MISSILE_ID = 10;
constexpr int AMMO_ID = 11;


extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern App* g_theApp;
extern RandomNumberGenerator* g_randGen;

//sounds
extern SoundID SOUND_ID_BULLET_SHOOT;
extern SoundID SOUND_ID_EXPLOSION;
extern SoundID SOUND_ID_HIT;
extern SoundID SOUND_ID_LOSE;
extern SoundID SOUND_ID_NEW_WAVE;
extern SoundID SOUND_ID_RESPAWN;
extern SoundID SOUND_ID_SPAWN_WAVE;
extern SoundID SOUND_ID_STARTUP;
extern SoundID SOUND_ID_WIN;
extern SoundID SOUND_ID_THRUST;
extern SoundID SOUND_ID_LASER;



void DebugDrawLine(Vec2 const& pointA, Vec2 const& pointB, float thickness, Rgba8 const& color);
void DebugDrawRing(Vec2 const& centerPos, float radius, float thickness, Rgba8 const& color);
void DebugDrawDisc(Vec2 const& centerPos, float radius, Rgba8 const& color);


