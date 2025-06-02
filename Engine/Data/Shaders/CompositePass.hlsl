//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD;
};

struct ps_output_t
{
    float4 colorRenderTarget : SV_Target0;
	float4 emissiveRenderTarget : SV_Target1;
};

//------------------------------------------------------------------------------------------------
Texture2D accumTexture : register(t0);
Texture2D revealageTexture : register(t1);
Texture2D emissiveAccumTexture : register(t2);

//------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	v2p.position = float4(input.localPosition, 1);
	v2p.uv = input.uv;
	return v2p;
}

//------------------------------------------------------------------------------------------------
ps_output_t PixelMain(v2p_t input)
{
	float revealage = (float)revealageTexture.Sample(diffuseSampler, input.uv);
	if (revealage == 1.f)
	{
		// Save the blending and color texture fetch cost
		discard;
	}

	float4 accumulation = accumTexture.Sample(diffuseSampler, input.uv);
	float4 emissiveAccumulation = emissiveAccumTexture.Sample(diffuseSampler, input.uv);

	float maxComponent = max( max(accumulation.r, accumulation.g),  accumulation.b);
    // Suppress overflow
    if (isinf(maxComponent))
	{
		accumulation.r = accumulation.a;
		accumulation.g = accumulation.a;
		accumulation.b = accumulation.a;
    }


	float3 averageColor = accumulation.rgb / max(accumulation.a, 1e-5);
	float3 averageEmissive = emissiveAccumulation.rgb / max(emissiveAccumulation.a, 1e-5);
	ps_output_t output;
	output.colorRenderTarget = float4(averageColor, 1.f - revealage);
	output.emissiveRenderTarget = float4(averageEmissive, 1.f - revealage);
	return output;
}
