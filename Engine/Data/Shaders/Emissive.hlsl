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
Texture2D<float> depthTexture : register(t8);

//------------------------------------------------------------------------------------------------
//SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);

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
	return v2p;
}

//------------------------------------------------------------------------------------------------
ps_output_t PixelMain(v2p_t input)
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;
	ParticleEmitterRenderDefinitionGPU renderDef = renderDefs[input.index];

	float4 color = textureColor * vertexColor * modelColor;
	clip(color.a - 0.01f);

	
	//panning texture
	float2 panTextureUVDims = renderDef.panTextureAtlasUVMaxs - renderDef.panTextureAtlasUVMins;
	float2 normalPanTextureUV = input.quadUV;
	normalPanTextureUV.x *= renderDef.panTextureSampleScale.x;
	normalPanTextureUV.y *= renderDef.panTextureSampleScale.y;
	normalPanTextureUV.x += input.particleLifetime * renderDef.panTextureSpeed.x;
	normalPanTextureUV.y += input.particleLifetime * renderDef.panTextureSpeed.y;
	normalPanTextureUV.x = normalPanTextureUV.x - (int)normalPanTextureUV.x;
	normalPanTextureUV.y = normalPanTextureUV.y - (int)normalPanTextureUV.y;
	if (normalPanTextureUV.x < 0.f)
	{
		normalPanTextureUV.x += 1.f;
	}
	if (normalPanTextureUV.y < 0.f)
	{
		normalPanTextureUV.y += 1.f;
	}
	float2 panTextureUV = normalPanTextureUV * panTextureUVDims + renderDef.panTextureAtlasUVMins;

	float4 panTextureColor = diffuseTexture.Sample(diffuseSampler, panTextureUV);
	//multiply color / largest component
	float colorScale = length(color.rgb);
	if (colorScale > 0.f && dot(panTextureColor.rgb, panTextureColor.rgb) > .00001f)
	{
		color.rgb = lerp( normalize(color.rgb), normalize(panTextureColor.rgb), input.panTextureContribution );
		color.rgb *= colorScale;
	}
	//we don't want to normalize colors that are 0,0,0 so just lerp instead
	else
	{
		color.rgb = lerp(color.rgb, panTextureColor.rgb, input.panTextureContribution );
	}

	// perform soft particle blending
    int2 pixelCoords = (int2)(input.position.xy);
    float depth0 = depthTexture.Load(int3(pixelCoords, 0));
    float surfaceLinearDepth = computeLinearDepth(depth0);
	float particleLinearDepth = computeLinearDepth(input.position.z);
	float softMultiplier = lerp(0.f, 1.f, (surfaceLinearDepth - particleLinearDepth));
	softMultiplier = clamp(softMultiplier, .0f, 1.f);
	softMultiplier = SmoothStart2(softMultiplier);
	softMultiplier = renderDef.softParticles == 0 ? 1.f : softMultiplier;
	color.a *= softMultiplier;
	color = float4(color.r * color.a, color.g * color.a, color.b * color.a, color.a);
	color = float4(color.r, color.g, color.b, color.a * input.alpha);

	ps_output_t output; 
	output.colorRenderTarget = color;

	//emissive value should not be multiplied into alpha value or things get weird
	output.emissiveRenderTarget = float4(input.emissive * color.rgb, color.a * input.alpha);

	return output;
}
