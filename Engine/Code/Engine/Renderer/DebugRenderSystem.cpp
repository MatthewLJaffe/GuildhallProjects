#include "DebugRenderSystem.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Window.hpp"
#include <mutex>

struct DebugRenderGeometry
{
	std::vector<Vertex_PCU> m_verts;
	Timer* m_timer;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	DebugRenderMode m_mode;
	RasterizeMode m_rasterizerMode;
	Texture* m_texture = nullptr;
	BlendMode blendMode = BlendMode::OPAQUE;
	BillboardType m_billlboardType = BillboardType::NONE;
	Vec3 m_billboardPos = Vec3(0.f,  0.f, 0.f);
};

struct DebugScreenText
{
	std::vector<Vertex_PCU> m_verts;
	Timer* m_timer;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	Texture* m_texture = nullptr;
	float m_textHeight = 0.f;
	std::string m_text;
};

class DebugRenderSystem
{
public:
	DebugRenderSystem(DebugRenderConfig config);
	std::vector<DebugRenderGeometry> m_debugRenderGeoms;
	std::vector<DebugScreenText> m_debugScreenTexts;
	std::vector<DebugScreenText> m_debugInfiniteMessageTexts;
	std::vector<DebugScreenText> m_debugFiniteMessageTexts;

	DebugRenderConfig m_config;
	void RenderGeometry(DebugRenderGeometry const& geom, Camera const& camera);
	void RenderText(DebugScreenText const& screenText,  Vec2 vertOffset = Vec2::ZERO);
	void InitializeGeomentry(DebugRenderGeometry& geom, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
	void InitializeScreenText(DebugScreenText& screenText, float duration, Rgba8 const& startColor, Rgba8 const& endColor);
	BitmapFont* font;
	int numInfiniteMessages = 0;
	int numTempMessages = 0;
	std::atomic<bool> m_hidden = false;
	std::mutex m_debugRenderMutex;
};

DebugRenderSystem* debugRenderSystem = nullptr;


void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	debugRenderSystem = new DebugRenderSystem(config);
	std::string fontFilePath = "Data/Fonts/";
	debugRenderSystem->font = config.m_renderer->CreateOrGetBitmapFontFromFile((fontFilePath + config.m_fontName).c_str());
	g_theEventSystem->SubscribeEventCallbackFunction("clear", Command_DebugRenderClear);
	g_theEventSystem->SubscribeEventCallbackFunction("toggle", Command_DebugRenderToggle);
}

void DebugRenderSystemShutdown()
{
	delete debugRenderSystem;
	debugRenderSystem = nullptr;
}

void DebugRenderSetVisisble()
{
	debugRenderSystem->m_hidden = false;
}

void DebugRenderSetHidden()
{
	debugRenderSystem->m_hidden = true;
}

void DebugRenderToggle()
{
	debugRenderSystem->m_hidden = !debugRenderSystem->m_hidden;
}

void DebugRenderClear()
{
	if (debugRenderSystem != nullptr)
	{
		debugRenderSystem->m_debugRenderMutex.lock();
		debugRenderSystem->m_debugRenderGeoms.clear();
		debugRenderSystem->m_debugScreenTexts.clear();
		debugRenderSystem->m_debugFiniteMessageTexts.clear();
		debugRenderSystem->m_debugInfiniteMessageTexts.clear();
		debugRenderSystem->m_debugRenderMutex.unlock();
	}
}

void DebugRemoveScreenText(std::string text)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	for (size_t i = 0; i < debugRenderSystem->m_debugScreenTexts.size(); i++)
	{
		if (debugRenderSystem->m_debugScreenTexts[i].m_text == text)
		{
			debugRenderSystem->m_debugScreenTexts.erase(debugRenderSystem->m_debugScreenTexts.begin() + i);
			break;
		}
	}
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugRemoveMessage(std::string text)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	for (size_t i = 0; i < debugRenderSystem->m_debugFiniteMessageTexts.size(); i++)
	{
		if (debugRenderSystem->m_debugFiniteMessageTexts[i].m_text == text)
		{
			debugRenderSystem->m_debugFiniteMessageTexts.erase(debugRenderSystem->m_debugFiniteMessageTexts.begin() + i);
			break;
		}
	}

	for (size_t i = 0; i < debugRenderSystem->m_debugInfiniteMessageTexts.size(); i++)
	{
		if (debugRenderSystem->m_debugInfiniteMessageTexts[i].m_text == text)
		{
			debugRenderSystem->m_debugInfiniteMessageTexts.erase(debugRenderSystem->m_debugInfiniteMessageTexts.begin() + i);
			break;
		}
	}
	debugRenderSystem->m_debugRenderMutex.unlock();
}



DebugRenderSystem::DebugRenderSystem(DebugRenderConfig config)
	: m_config(config)
{}

void DebugRenderSystem::RenderGeometry(DebugRenderGeometry const& geom, Camera const& camera)
{
	bool locked = m_debugRenderMutex.try_lock();
	
	m_config.m_renderer->BindTexture(geom.m_texture);
	m_config.m_renderer->SetRasterizeMode(geom.m_rasterizerMode);
	Mat44 identity;
	Rgba8 currColor = LerpColor(geom.m_startColor, geom.m_endColor, geom.m_timer->GetElapsedFraction());
	if (geom.m_billlboardType == BillboardType::FULL_OPPOSING)
	{
		Mat44 targetMatrix = camera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
		targetMatrix = camera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
		targetMatrix.AppendZRotation(90.f);
		targetMatrix.AppendTranslation3D(camera.m_position);

		Mat44 billboardMatrix = GetBillboardMatrix(BillboardType::FULL_OPPOSING, targetMatrix, geom.m_billboardPos);
		m_config.m_renderer->SetModelConstants(billboardMatrix, currColor);
	}
	else
	{
		m_config.m_renderer->SetModelConstants(identity, currColor);
	}
	m_config.m_renderer->SetBlendMode(geom.blendMode);
	if (geom.m_mode == DebugRenderMode::ALWAYS)
	{
		m_config.m_renderer->SetDepthMode(DepthMode::DISABLED);
		m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
		m_config.m_renderer->BindShader(nullptr);
		m_config.m_renderer->DrawVertexArray(geom.m_verts.size(), geom.m_verts.data());
	}
	if (geom.m_mode == DebugRenderMode::USE_DEPTH)
	{
		m_config.m_renderer->SetDepthMode(DepthMode::ENABLED);
		m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
		m_config.m_renderer->BindShader(nullptr);
		m_config.m_renderer->DrawVertexArray(geom.m_verts.size(), geom.m_verts.data());
	}
	if (geom.m_mode == DebugRenderMode::X_RAY)
	{
		m_config.m_renderer->SetDepthMode(DepthMode::DISABLED);
		m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
		m_config.m_renderer->SetModelConstants(identity, Rgba8(currColor.r, currColor.g, currColor.b, 100));
		m_config.m_renderer->DrawVertexArray(geom.m_verts.size(), geom.m_verts.data());

		m_config.m_renderer->SetDepthMode(DepthMode::ENABLED);
		m_config.m_renderer->SetBlendMode(BlendMode::OPAQUE);
		m_config.m_renderer->SetModelConstants(identity, currColor);
		m_config.m_renderer->BindShader(nullptr);
		m_config.m_renderer->DrawVertexArray(geom.m_verts.size(), geom.m_verts.data());
	}
	if (locked)
	{
		m_debugRenderMutex.unlock();
	}
}

void DebugRenderSystem::RenderText(DebugScreenText const& screenText, Vec2 offset)
{
	bool locked = m_debugRenderMutex.try_lock();
	
	Rgba8 currColor = LerpColor(screenText.m_startColor, screenText.m_endColor, screenText.m_timer->GetElapsedFraction());
	m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
	m_config.m_renderer->SetDepthMode(DepthMode::DISABLED);
	m_config.m_renderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	m_config.m_renderer->BindTexture(screenText.m_texture);
	m_config.m_renderer->BindShader(nullptr);
	Mat44 textTransForm;
	textTransForm.AppendTranslation2D( offset );
	m_config.m_renderer->SetModelConstants(textTransForm, currColor);
	m_config.m_renderer->DrawVertexArray(screenText.m_verts.size(), screenText.m_verts.data());
	if (locked)
	{
		m_debugRenderMutex.unlock();
	}
}

void DebugRenderSystem::InitializeGeomentry(DebugRenderGeometry& geom, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	bool locked = m_debugRenderMutex.try_lock();
	
	//infinite duration
	if (duration == -1)
	{
		duration = 99999999999.f;
		geom.m_endColor = geom.m_startColor;
		geom.m_timer = new Timer(duration);
		geom.m_timer->Start();
	}
	//1 frame duration
	if (duration == 0.f)
	{
		geom.m_timer = new Timer(1.f);
		geom.m_timer->Start();
		geom.m_timer->m_startTime -= 1.f;
	}

	geom.m_timer = new Timer(duration);
	geom.m_timer->Start();

	geom.m_startColor = startColor;
	geom.m_endColor = endColor;
	geom.m_mode = mode;
	if (locked)
	{
		m_debugRenderMutex.unlock();
	}
}

void DebugRenderSystem::InitializeScreenText(DebugScreenText& screenText, float duration, Rgba8 const& startColor, Rgba8 const& endColor)
{
	bool locked = m_debugRenderMutex.try_lock();
	
	//infinite duration
	if (duration == -1)
	{
		duration = 99999999999.f;
		screenText.m_endColor = screenText.m_startColor;
		screenText.m_timer = new Timer(duration);
		screenText.m_timer->Start();
	}
	//1 frame duration
	if (duration == 0.f)
	{
		screenText.m_timer = new Timer(1.f);
		screenText.m_timer->Start();
		screenText.m_timer->m_startTime -= 1.f;
	}

	screenText.m_timer = new Timer(duration);
	screenText.m_timer->Start();

	screenText.m_startColor = startColor;
	screenText.m_endColor = endColor;
	if (locked)
	{
		m_debugRenderMutex.unlock();
	}
}

void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugRenderGeometry geom;
	AddVertsForSphere3D(geom.m_verts, pos, radius, startColor);
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_rasterizerMode = RasterizeMode::SOLID_CULL_BACK;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugRenderGeometry geom;
	AddVertsForCylinder3D(geom.m_verts, start, end, radius, startColor, AABB2::ZERO_TO_ONE, 32);
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_rasterizerMode = RasterizeMode::SOLID_CULL_NONE;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddWorldAABB3(const Vec3& mins, const Vec3& maxs, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugRenderGeometry geom;
	AddVertsForAABB3D(geom.m_verts,AABB3(mins, maxs), startColor, AABB2::ZERO_TO_ONE);
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_rasterizerMode = RasterizeMode::SOLID_CULL_NONE;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddWorldBoxLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugRenderGeometry geom;
	AddVertsForBoxLine3D(geom.m_verts, start, end, radius, startColor);
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_rasterizerMode = RasterizeMode::SOLID_CULL_NONE;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugRenderGeometry geom;
	AddVertsForCylinder3D(geom.m_verts, base, top, radius, startColor);
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_rasterizerMode = RasterizeMode::WIREFRAME_CULL_NONE;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddWorldWireSphere(Vec3 const& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugRenderGeometry geom;
	AddVertsForSphere3D(geom.m_verts, center, radius, startColor, AABB2::ZERO_TO_ONE, 16);
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_rasterizerMode = RasterizeMode::WIREFRAME_CULL_NONE;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	bool locked = debugRenderSystem->m_debugRenderMutex.try_lock();
	
	Vec3 endToStartDir = (start - end).GetNormalized();
	Vec3 cylinderEndPos = end + (endToStartDir * 3.f * radius);
	DebugRenderGeometry geom;
	AddVertsForCylinder3D(geom.m_verts, start, cylinderEndPos, radius, startColor, AABB2::ZERO_TO_ONE, 32);
	std::vector<Vertex_PCU> coneVerts;
	AddVertsForCone3D(coneVerts, cylinderEndPos, end, radius * 1.75f, startColor, AABB2::ZERO_TO_ONE, 32);
	for (size_t i = 0; i < coneVerts.size(); i++)
	{
		geom.m_verts.push_back(coneVerts[i]);
	}
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_rasterizerMode = RasterizeMode::SOLID_CULL_NONE;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	if (locked)
	{
		debugRenderSystem->m_debugRenderMutex.unlock();
	}
}

void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugRenderGeometry geom;
 	debugRenderSystem->font->AddVertsForText3DAtOriginXForward(geom.m_verts, textHeight, text, startColor, 1.f, alignment);
	TransformVertexArray3D(geom.m_verts, transform);
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_rasterizerMode = RasterizeMode::SOLID_CULL_NONE;
	geom.m_texture = debugRenderSystem->font->GetTexture();
	geom.blendMode = BlendMode::ALPHA;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, 
	float textHeight, const Vec2& alignment, 
	float duration, const Rgba8& startColor, 
	const Rgba8 endColor, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugRenderGeometry geom;
	debugRenderSystem->font->AddVertsForText3DAtOriginXForward(geom.m_verts, textHeight, text, startColor, 1.f, alignment);
	debugRenderSystem->InitializeGeomentry(geom, duration, startColor, endColor, mode);
	geom.m_billlboardType = BillboardType::FULL_OPPOSING;
	geom.m_billboardPos = origin;
	geom.m_rasterizerMode = RasterizeMode::SOLID_CULL_NONE;
	geom.m_texture = debugRenderSystem->font->GetTexture();
	geom.blendMode = BlendMode::ALPHA;

	debugRenderSystem->m_debugRenderGeoms.push_back(geom);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddScreenText(const std::string& text, const Vec2& position, float size, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugScreenText screenText;
	screenText.m_textHeight = size;
	screenText.m_text = text;
	float textWidth = (float)text.size() * size;
	Vec2 textMins(Lerp(-textWidth, 0.f, alignment.x), Lerp(-size, 0.f, alignment.y));
	textMins += position;
	debugRenderSystem->font->AddVertsForText2D(screenText.m_verts, textMins, size, text, Rgba8::WHITE, 2.f / Window::GetTheWindowInstance()->GetAspectRatio());
	debugRenderSystem->InitializeScreenText(screenText, duration, startColor, endColor);
	screenText.m_texture = debugRenderSystem->font->GetTexture();

	debugRenderSystem->m_debugScreenTexts.push_back(screenText);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddMessage(const std::string& text, float height, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	DebugScreenText screenText;
	screenText.m_text = text;
	Vec2 textMins(0.f, -height * 1.1f);
	screenText.m_textHeight = height;
	debugRenderSystem->font->AddVertsForText2D(screenText.m_verts, textMins, height, text, Rgba8::WHITE, 2.f / Window::GetTheWindowInstance()->GetAspectRatio());
	debugRenderSystem->InitializeScreenText(screenText, duration, startColor, endColor);
	screenText.m_texture = debugRenderSystem->font->GetTexture();

	if (duration == -1.f)
	{
		debugRenderSystem->m_debugInfiniteMessageTexts.push_back(screenText);
	}
	else
	{
		debugRenderSystem->m_debugFiniteMessageTexts.push_back(screenText);
	}
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugAddWorldBasis(const Mat44& transform, float duration, DebugRenderMode mode)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	float uniformScale = transform.GetIBasis3D().GetLength();
	DebugAddWorldArrow(transform.GetTranslation3D(), transform.GetTranslation3D() + transform.GetIBasis3D(), .05f * uniformScale, duration, Rgba8::RED, Rgba8::RED, mode);
	DebugAddWorldArrow(transform.GetTranslation3D(), transform.GetTranslation3D() + transform.GetJBasis3D(), .05f * uniformScale, duration, Rgba8::GREEN, Rgba8::GREEN, mode);
	DebugAddWorldArrow(transform.GetTranslation3D(), transform.GetTranslation3D() + transform.GetKBasis3D(), .05f * uniformScale, duration, Rgba8::BLUE, Rgba8::BLUE, mode);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

bool Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args);
	DebugRenderClear();
	return false;
}

bool Command_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args);
	DebugRenderToggle();
	return false;
}

void DebugRenderBeginFrame()
{
	
}

void DebugRenderWorld(const Camera& camera)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	debugRenderSystem->m_config.m_renderer->BeginCamera(camera);
	for (size_t i = 0; i < debugRenderSystem->m_debugRenderGeoms.size(); i++)
	{
		if (!debugRenderSystem->m_hidden)
		{
			debugRenderSystem->RenderGeometry(debugRenderSystem->m_debugRenderGeoms[i], camera);
		}
		if (debugRenderSystem->m_debugRenderGeoms[i].m_timer->HasPeriodElapsed())
		{
			debugRenderSystem->m_debugRenderGeoms.erase(debugRenderSystem->m_debugRenderGeoms.begin() + i);
			i--;
		}
	}
	debugRenderSystem->m_config.m_renderer->EndCamera(camera);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugRenderScreen(const Camera& camera)
{
	debugRenderSystem->m_debugRenderMutex.lock();
	debugRenderSystem->m_config.m_renderer->BeginCamera(camera);
	for (size_t i = 0; i < debugRenderSystem->m_debugScreenTexts.size(); i++)
	{
		if (!debugRenderSystem->m_hidden)
		{
			debugRenderSystem->RenderText(debugRenderSystem->m_debugScreenTexts[i]);
		}

		if (debugRenderSystem->m_debugScreenTexts[i].m_timer->HasPeriodElapsed())
		{
			debugRenderSystem->m_debugScreenTexts.erase(debugRenderSystem->m_debugScreenTexts.begin() + i);
			i--;
		}
	}

	Vec2 offset(camera.GetOrthoBottomLeft().x, camera.GetOrthoTopRight().y);
	for (size_t i = 0; i < debugRenderSystem->m_debugInfiniteMessageTexts.size(); i++)
	{
		if (!debugRenderSystem->m_hidden)
		{
			debugRenderSystem->RenderText(debugRenderSystem->m_debugInfiniteMessageTexts[i], offset);
		}

		offset.y -= debugRenderSystem->m_debugInfiniteMessageTexts[i].m_textHeight;
		if (debugRenderSystem->m_debugInfiniteMessageTexts[i].m_timer->HasPeriodElapsed())
		{
			debugRenderSystem->m_debugInfiniteMessageTexts.erase(debugRenderSystem->m_debugInfiniteMessageTexts.begin() + i);
			i--;
		}
	}
	for (size_t i = 0; i < debugRenderSystem->m_debugFiniteMessageTexts.size(); i++)
	{
		if (!debugRenderSystem->m_hidden)
		{
			debugRenderSystem->RenderText(debugRenderSystem->m_debugFiniteMessageTexts[i], offset);
		}

		offset.y -= debugRenderSystem->m_debugFiniteMessageTexts[i].m_textHeight;
		if (debugRenderSystem->m_debugFiniteMessageTexts[i].m_timer->HasPeriodElapsed())
		{
			debugRenderSystem->m_debugFiniteMessageTexts.erase(debugRenderSystem->m_debugFiniteMessageTexts.begin() + i);
			i--;
		}
	}
	debugRenderSystem->m_config.m_renderer->EndCamera(camera);
	debugRenderSystem->m_debugRenderMutex.unlock();
}

void DebugRenderEndFrame()
{
}
