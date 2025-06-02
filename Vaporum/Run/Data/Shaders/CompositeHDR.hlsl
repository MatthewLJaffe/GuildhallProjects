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

cbuffer HDRConstants : register(b7)
{
	float c_exposure;
	float3 padding;
}

//------------------------------------------------------------------------------------------------
Texture2D hdrTexture : register(t0);

//------------------------------------------------------------------------------------------------
SamplerState hdrSampler : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	v2p.position = float4(input.localPosition, 1);
	v2p.uv = input.uv;
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	const float gamma = 2.2f;
	float3 hdrColor = hdrTexture.Sample(hdrSampler, input.uv).rgb;
	if (hdrColor.r <= 1.f && hdrColor.g <= 1.f && hdrColor.b <= 1.f)
	{
		//return float4(hdrColor, 1.f);
	}
	//exposure tone mapping
	float3 mapped = float3(1.f, 1.f, 1.f) - exp(-hdrColor * c_exposure);
	float invGamma = 1.f/gamma;
	mapped = pow(mapped, float3(invGamma, invGamma, invGamma));

	return float4(mapped, 1.f);
}
