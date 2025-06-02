#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Game/ModelDefinition.hpp"


class Model : public Entity
{
public:
	Model(Vec3 const& position, std::string const& filePath);
	virtual ~Model();

	void Update(float deltaSeconds);
	void Render() const override;
	ModelDefinition const* GetDef() const;
protected:
	int m_defIndex = -1;
};