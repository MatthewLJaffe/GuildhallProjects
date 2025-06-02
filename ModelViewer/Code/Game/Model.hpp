#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"

class Model : public Entity
{
public:
	Model(Game* game, Vec3 const& position);
	virtual ~Model();

	bool Load(std::string const& fileName);

	void Update(float deltaSeconds);
	void Render() const override;
protected:
	bool LoadXml(std::string const& fileName);
	bool LoadObj(std::string const&, Mat44 const& transform = Mat44());


	std::string m_name;
	std::string m_objFilename;
	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh* m_gpuMesh = nullptr;
	Material* m_material = nullptr;
	Mat44 m_modelTransform;
};