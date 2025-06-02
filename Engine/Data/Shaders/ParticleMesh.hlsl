#include "ParticleCommon.h"

//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	uint index : INDEX;
	float2 quadUV : QUAD_TEXCOORD;
	float emissive : EMISSIVE;
	float alpha : ALPHA;
	float panTextureContribution : PAN_TEXTURE_CONTRIBUTION;
	float particleLifetime : PARTICLE_LIFETIME;
	float3 localNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	uint index : INDEX;
	float2 quadUV : QUAD_TEXCOORD;
	float emissive : EMISSIVE;
	float alpha : ALPHA;
	float panTextureContribution : PAN_TEXTURE_CONTRIBUTION;
	float particleLifetime : PARTICLE_LIFETIME;
	float3 normal : NORMAL;
};

struct ps_output_t
{
	float4 colorRenderTarget : SV_Target0;
	float4 emissiveRenderTarget : SV_Target1;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
StructuredBuffer<ParticleEmitterRenderDefinitionGPU> renderDefs : register(t1);
StructuredBuffer<FloatGraph> floatGraphs : register(t2);
StructuredBuffer<Float2Graph> float2Graphs : register(t3);
StructuredBuffer<Float3Graph> float3Graphs : register(t4);

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	float4 worldNormal = mul(ModelMatrix, float4(input.localNormal, 0));

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.index = input.index;
	v2p.quadUV = input.quadUV;
	v2p.emissive = input.emissive;
	v2p.alpha = input.alpha;
	v2p.panTextureContribution = input.panTextureContribution;
	v2p.particleLifetime = input.particleLifetime;
	v2p.normal = (float3)worldNormal;
	return v2p;
}

//------------------------------------------------------------------------------------------------
ps_output_t PixelMain(v2p_t input)
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;
	ParticleEmitterRenderDefinitionGPU renderDef = renderDefs[input.index];

	float ambient = AmbientIntensity;
	float diffuse = SunIntensity * saturate(dot(normalize(input.normal), -SunDirection));	
	float totalLight = saturate( ambient + diffuse );
	float4 lightColor = float4(totalLight.xxx, 1);

	float4 color = textureColor * vertexColor * modelColor * lightColor;
	clip(color.a - 0.01f);

	/*
	//panning texture
	float2 panTextureUVDims = renderDef.panTextureAtlasUVMaxs - renderDef.panTextureAtlasUVMins;
	float2 panTextureUV = (input.quadUV * panTextureUVDims) + renderDef.panTextureAtlasUVMins;
	panTextureUV.x *= renderDef.panTextureSampleScale.x;
	panTextureUV.y *= renderDef.panTextureSampleScale.y;
	panTextureUV.x += input.particleLifetime * renderDef.panTextureSpeed.x;
	panTextureUV.y += input.particleLifetime * renderDef.panTextureSpeed.y;
	panTextureUV = abs(panTextureUV);
	panTextureUV.x = panTextureUV.x - (int)panTextureUV.x;
	panTextureUV.y = panTextureUV.y - (int)panTextureUV.y;
	float4 panTextureColor = diffuseTexture.Sample(diffuseSampler, panTextureUV);
	//multiply color / largest component
	float colorScale = length(color.rgb);
	color.rgb = lerp( normalize(color.rgb), normalize(panTextureColor.rgb), input.panTextureContribution );
	color.rgb *= colorScale;
	*/

	color = float4(color.r * color.a, color.g * color.a, color.b * color.a, color.a);
	color = float4(color.r, color.g, color.b, color.a * input.alpha);

	ps_output_t output;
	output.colorRenderTarget = color;

	//emissive value should not be multiplied into alpha value or things get weird
	output.emissiveRenderTarget = float4(input.emissive * color.rgb, color.a * input.alpha);
	
	return output;
}
