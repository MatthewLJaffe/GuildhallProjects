#include "Engine/Renderer/GPUBuffer.hpp"
struct ID3D11Buffer;

class VertexBuffer : public GPUBuffer
{
	friend class Renderer;

public:
	VertexBuffer(const VertexBuffer& copy) = delete;
	virtual ~VertexBuffer() override;

private:
	VertexBuffer(size_t size);
};