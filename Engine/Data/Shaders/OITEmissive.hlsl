//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	uint index : INDEX;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	uint index : INDEX;
};

struct ps_output_t
{
    float4 colorRenderTarget : SV_Target0;
	float4 emissiveRenderTarget : SV_Target1;
	float4 accumulationTarget : SV_Target2;
	float revealageTarget : SV_Target3;
	float4 emissiveAccumulationTarget : SV_Target4;
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);

//------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);

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
	v2p.emissive = input.emissive;
	return v2p;
}

//------------------------------------------------------------------------------------------------
ps_output_t PixelMain(v2p_t input)
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 fragmentColor = input.color;
	float4 modelColor = ModelColor;
	float totalLight = saturate(input.emissive);
	float4 lightColor = float4(totalLight.xxx, 1);
	float4 color = textureColor * fragmentColor * modelColor;
	clip(color.a - 0.01f);
	ps_output_t output;
	//do not write to color render target since not opaque
	output.colorRenderTarget = float4(0.f, 0.f, 0.f, 0.f);
	float4 emissiveColor =  lightColor * color;
	
	//write to accumulation and revealage render targets
	float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 100000000.0 * pow(1.0 - input.position.z * 0.9, 3.0), 0.01, 3000.0);
	/*
	float weight =
		max(min(1.0, max(max(color.r, color.g), color.b) * color.a), color.a) *
		clamp(0.03 / (1e-5 + pow(input.position.z / 200.f, 4.0)), 1e-2, 3e3);
	*/
	//float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - input.position.z * 0.9, 3.0), 1e-2, 3e3);
	/*
	float a = min(1.0, color.a) * 8.0 + 0.01;
    float b = -input.position.z * 0.95 + 1.0;
    float weight    = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
	*/

	output.accumulationTarget = float4(color.rgb * color.a, color.a) * weight;
	output.emissiveRenderTarget = float4(0.f, 0.f, 0.f, 0.f);
	output.emissiveAccumulationTarget = float4(emissiveColor.rgb * emissiveColor.a, emissiveColor.a) * weight;
	output.revealageTarget = color.a;
	return output;
}
