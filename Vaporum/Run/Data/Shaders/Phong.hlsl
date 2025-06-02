struct vs_input_t
{
    float3 localPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float3 localTangent : TANGENT;
    float3 localBitangent : BITANGENT;
    float3 localNormal : NORMAL;
};

struct v2p_t
{
    float4 clipPosition : SV_Position;
    float4 worldPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 normal : NORMAL;
};

struct ps_output_t
{
	float4 colorRenderTarget : SV_Target0;
	float4 emissiveRenderTarget : SV_Target1;

};



//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 padding;
	float3 worldEyePosition;
	float minimumFalloff;
	float maximumFalloff;
	float minimumFalloffMultiplier;
	float maximumFalloffMultiplier;
	int renderAmbientDebugFlag;
	int renderDiffuseDebugFlag;
	int renderSpecularDebugFlag;
	int renderEmissiveDebugFlag;
	int useDiffuseMapDebugFlag;
	int useNormalMapDebugFlag;
	int useSpecularMapDebugFlag;
	int useGlossinessMapDebugFlag;
	int useEmissiveMapDebugFlag;
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
Texture2D specGlossEmitTexture : register(t1);
Texture2D normalMapTexture : register(t2);


//------------------------------------------------------------------------------------------------
SamplerState sampleState : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	float4 localNormal = float4(input.localNormal, 0);
	float4 localTangent = float4(input.localTangent, 0);
	float4 localBitangent = float4(input.localBitangent, 0);
	float4 worldTangent= mul(ModelMatrix, localTangent);
	float4 worldBitangent = mul(ModelMatrix, localBitangent);
	float4 worldNormal = mul(ModelMatrix, localNormal);

	v2p_t v2p;
	v2p.worldPosition = worldPosition;
	v2p.clipPosition = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.tangent = worldTangent;
	v2p.bitangent = worldBitangent;
	v2p.normal = worldNormal;
	return v2p;
}

//------------------------------------------------------------------------------------------------
ps_output_t PixelMain(v2p_t input)
{
	float4 modelColor = ModelColor;
	float4 vertexColor = input.color;
	float4 textureColor = float4(1.f, 1.f, 1.f, 1.f);
	if (useDiffuseMapDebugFlag == 1)
	{
		 textureColor = diffuseTexture.Sample(sampleState, input.uv);
	}

	float3 vertexWorldNormal = normalize(input.normal.xyz);
	float3 pixelWorldNormal = vertexWorldNormal;
	if (useNormalMapDebugFlag == 1)
	{
		float4 sampleNormal = normalMapTexture.Sample(sampleState, input.uv);
		sampleNormal.x = sampleNormal.x * 2.f - 1.f;
		sampleNormal.y = sampleNormal.y * 2.f - 1.f;
		sampleNormal.z = sampleNormal.z * 2.f - 1.f;

		float3 tangent = normalize(input.tangent);
		float3 bitangent = normalize(input.bitangent);
		float3 normal = normalize(input.normal);

		float4x4 tbnMatrix = {	tangent.x, bitangent.x, normal.x, 0.f,
								tangent.y, bitangent.y, normal.y, 0.f,
								tangent.z, bitangent.z, normal.z, 0.f, 
								0.f, 0.f, 0.f, 1.f
							 };
		pixelWorldNormal = mul(tbnMatrix, sampleNormal).xyz;
		
	}

	float ambient = AmbientIntensity;
	float diffuse = SunIntensity * saturate(dot(normalize(pixelWorldNormal), -SunDirection));

	float specularIntensity = 1.f;
	float specularPower = 32.f;
	
	float4 specGlossEmitColor = specGlossEmitTexture.Sample(sampleState, input.uv);
	if (useSpecularMapDebugFlag)
	{
		specularIntensity = specGlossEmitColor.r;
		specularPower = specGlossEmitColor.g * 31.f + 1.f;
	}
	float emissive = .0f;
	if ( useEmissiveMapDebugFlag && renderEmissiveDebugFlag)
	{
		emissive = specGlossEmitColor.b;
	}
	
	float specular; // specular light intensity after performing the following calculations
	float3 worldViewDirection = normalize(worldEyePosition - input.worldPosition);
	float3 worldHalfVector = normalize(-SunDirection + worldViewDirection);
	float specularDot = saturate( dot(worldHalfVector, pixelWorldNormal) );
	specular = specularIntensity * pow(specularDot, specularPower);
	
	ambient *= renderAmbientDebugFlag == 1 ? 1.f : 0.f;
	diffuse *= renderDiffuseDebugFlag == 1 ? 1.f : 0.f;
	specular *= renderSpecularDebugFlag == 1 ? 1.f : 0.f;

	float totalLight = saturate( ambient + diffuse + specular + emissive);
	float4 lightColor = float4(totalLight.xxx, 1);
	float4 color = lightColor * textureColor * vertexColor * modelColor;
	clip(color.a - 0.01f);

	ps_output_t output;
	output.colorRenderTarget = color;
	output.emissiveRenderTarget = float4(emissive.xxx, 1.f) * textureColor * vertexColor * modelColor;
	return output;
}
