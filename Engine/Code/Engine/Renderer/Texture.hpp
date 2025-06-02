#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;

class Texture
{
	friend class Renderer; // Only the Renderer can create new Texture objects!

private:
	Texture() = default; // can't instantiate directly; must ask Renderer to do it for you
	~Texture();

public:
	Texture(Texture const& copy) = delete; // No copying allowed!  This represents GPU memory.
	IntVec2				GetDimensions() const { return m_dimensions; }
	std::string const& GetImageFilePath() const { return m_name; }
public:
	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
protected:
	std::string			m_name;
	IntVec2				m_dimensions;
};