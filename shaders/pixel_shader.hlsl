SamplerState texSampler : register(s0);
SamplerState cubeSampler : register(s1);
Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
TextureCube textureCube : register(t2);

cbuffer MaterialBuffer : register(b1)
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
};

cbuffer LightBuffer : register(b2)
{
	float4 LightSourcePosition;
	float4 CameraPosition;
};

struct PSIn
{
	float4 Pos  : SV_Position;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
	float2 TexCoord : TEX;
	float3 PosWorld : WORLDPOS;
};

//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------

float4 PS_main(PSIn input) : SV_Target
{
	float3 A, D, S;
	
	float3 T = normalize(input.Tangent);
	float3 B = normalize(input.Binormal);
	float3 N = normalize(input.Normal);
	float3x3 TBN = transpose(float3x3(T, B, N));
	
	float4 normalVector = texNormal.Sample(texSampler, input.TexCoord);
	float4 diffuseColor = texDiffuse.Sample(texSampler, input.TexCoord);
	float3 newNormal = mul(TBN, normalVector.xyz * 2 - 1);

	float3 lightVector = normalize(LightSourcePosition.xyz - input.PosWorld);
	float3 viewVector = normalize(CameraPosition.xyz - input.PosWorld);
	float3 reflection = normalize(reflect(lightVector, newNormal));

	A = Ambient.xyz * 0.25f;
    D = max(mul(diffuseColor.xyz, dot(lightVector, newNormal)), 0);
	S = mul(Specular.xyz, pow(max(dot(reflection, viewVector), 0), 50));

	// Debug shading #1: map and return normal as a color, i.e. from [-1,1]->[0,1] per component
	// The 4:th component is opacity and should be = 1
	//return float4(input.Normal*0.5+0.5, 1);

	//return float4(A + D + S, 1);
	
    //return float4(diffuseColor.xyz, 1);
	
    return float4(newNormal, 0) * 0.5 + 0.5;

	//return float4(A, 1);

	//return float4(D, 1);

	//return float4(S, 1);
	
	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
//	return float4(input.TexCoord, 0, 1);
}