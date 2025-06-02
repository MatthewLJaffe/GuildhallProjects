//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 localTangent : TANGENT;
	float3 localBitangent : BITANGENT;
	float3 localNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 tangent : TANGENT;
	float4 bitangent : BITANGENT;
	float4 normal : NORMAL;
	float3 worldPosition : POSITION;
};

struct PointLight
{
	float3 position;
	float intensity;
	float linearAttenuation;
	float exponentialAttenuation;
	float2 padding;
	float4 color;
};

//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 padding1;
	//PointLight ConstantPointLight;
	//float2 padding2;
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

cbuffer PointLights : register(b4)
{
	uint numPointLights = 0;
	float3 pointLightPadding;
	PointLight pointLights[512];
}

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
	float4 localNormal = float4(input.localNormal, 0);
	float4 worldNormal = mul(ModelMatrix, localNormal);

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.tangent = float4(0, 0, 0, 0);
	v2p.bitangent = float4(0, 0, 0, 0);
	v2p.normal = worldNormal;
	v2p.worldPosition = worldPosition;
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float ambient = AmbientIntensity;
	float directional = SunIntensity * saturate(dot(normalize(input.normal.xyz), -SunDirection));

	float4 pointLightContribution = float4(0.f, 0.f, 0.f, 0.f);
	for (uint i = 0; i < numPointLights; i++)
	{
		PointLight currPointLight = pointLights[i];
	
		float3 dispFromPosToPoint = currPointLight.position - input.worldPosition;
		float distanceToPoint = length(dispFromPosToPoint);
		float3 pointLightDir = normalize(dispFromPosToPoint);
		float pointLightBase = saturate(dot(normalize(input.normal.xyz), pointLightDir)) * currPointLight.intensity;
		float attenuation = 1.f +
			currPointLight.linearAttenuation * distanceToPoint +
			currPointLight.exponentialAttenuation * distanceToPoint * distanceToPoint;
		pointLightContribution += currPointLight.color * (pointLightBase / attenuation);
	}

	float4 lightColor = float4((ambient + directional).xxx, 1) + pointLightContribution;
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;
	float4 color = lightColor * textureColor * vertexColor * modelColor;
	clip(color.a - 0.01f);
	return color;
}
