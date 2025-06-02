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
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 worldPosition : POSITION;
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
cbuffer SimpleMinerConstants : register(b5)
{
	float3 CameraWorldPos;
	float padding;
	float4 IndoorLightColor;
	float4 OutdoorLightColor;
	float4 SkyColor;
	float FogFarDistance;
	float FogNearDistance;
	float2 padding2;
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
	v2p.uv = input.uv;
	v2p.worldPosition = worldPosition;

	float4 outdoorContribution = input.color.r * OutdoorLightColor;
	float4 indoorContribution = input.color.g * IndoorLightColor;
	v2p.color = float4(1,1,1,1) - (float4(1,1,1,1) - outdoorContribution) * (float4(1,1,1,1) - indoorContribution);
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;
	float4 color = textureColor * vertexColor * modelColor;

	float distanceFromCamera = distance(CameraWorldPos, input.worldPosition.xyz);
	float fractionFromFar = (distanceFromCamera - FogNearDistance) / (FogFarDistance - FogNearDistance);
	fractionFromFar = saturate(fractionFromFar);
	float4 outputColor = lerp(color, SkyColor, fractionFromFar);
	//float4 outputColor = lerp(color, color, fractionFromFar);
	return outputColor;
}
