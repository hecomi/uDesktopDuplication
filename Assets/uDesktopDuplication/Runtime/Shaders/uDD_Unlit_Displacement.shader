Shader "uDesktopDuplication/Unlit_Displacement"
{

Properties
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _ColorScale ("ColorScale", Range(0.0, 10.0)) = 1.0
    _MainTex ("Texture", 2D) = "white" {}
    [KeywordEnum(Y, Z)] _Forward("Mesh Forward Direction", Int) = 0
    [Toggle(BEND_ON)] _Bend("Use Bend", Int) = 0
    [PowerSlider(10.0)] _Radius("Bend Radius", Range(1, 100)) = 30
    _Width ("Width", Range(0.0, 10.0)) = 1.92
    [PowerSlider(10.0)] _Thickness("Thickness", Range(0.01, 10)) = 1
    [KeywordEnum(Off, Front, Back)] _Cull("Culling", Int) = 2
    _DispTex ("Displacement Map", 2D) = "black" {}
    _DispFactor("Displacement Factor", Range(0, 5.0)) = 1
    _TessMinDist("Tessellation Min Distance", Range(0.1, 100.0)) = 1.0
    _TessMaxDist("Tessellation Max Distance", Range(0.1, 100.0)) = 5.0
    _TessFactor("Tessellation Factor", Range(0.1, 50.0)) = 10.0
}

SubShader
{

Tags { "RenderType"="Opaque" }

Cull [_Cull]

CGINCLUDE

#include "./uDD_Common.cginc"
#include "Tessellation.cginc"

Texture2D _DispTex;
SamplerState sampler_DispTex;
half _DispFactor;
half _TessMinDist;
half _TessMaxDist;
half _TessFactor;

struct VsInput
{
    float3 vertex   : POSITION;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct HsInput
{
    float4 f4Position : POS;
    float3 f3Normal   : NORMAL;
    float2 f2TexCoord : TEXCOORD;
};

struct HsControlPointOutput
{
    float3 f3Position : POS;
    float3 f3Normal   : NORMAL;
    float2 f2TexCoord : TEXCOORD;
};

struct HsConstantOutput
{
    float fTessFactor[3]    : SV_TessFactor;
    float fInsideTessFactor : SV_InsideTessFactor;
};

struct DsOutput
{
    float4 f4Position : SV_Position;
    float2 f2TexCoord : TEXCOORD0;
};

HsInput vert(VsInput i)
{
    HsInput o;
    o.f4Position = float4(i.vertex, 1.0);
    o.f3Normal   = i.normal;
    o.f2TexCoord = i.texcoord;
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[patchconstantfunc("hullConst")]
[outputcontrolpoints(3)]
HsControlPointOutput hull(InputPatch<HsInput, 3> i, uint id : SV_OutputControlPointID)
{
    HsControlPointOutput o = (HsControlPointOutput)0;
    o.f3Position = i[id].f4Position.xyz;
    o.f3Normal   = i[id].f3Normal;
    o.f2TexCoord = i[id].f2TexCoord;
    return o;
}

HsConstantOutput hullConst(InputPatch<HsInput, 3> i)
{
    HsConstantOutput o = (HsConstantOutput)0;
    
    float4 p0 = i[0].f4Position;
    float4 p1 = i[1].f4Position;
    float4 p2 = i[2].f4Position;
    float4 tessFactor = UnityDistanceBasedTess(p0, p1, p2, _TessMinDist, _TessMaxDist, _TessFactor);

    o.fTessFactor[0] = tessFactor.x;
    o.fTessFactor[1] = tessFactor.y;
    o.fTessFactor[2] = tessFactor.z;
    o.fInsideTessFactor = tessFactor.w;
           
    return o;
}

[domain("tri")]
DsOutput domain(
    HsConstantOutput hsConst, 
    const OutputPatch<HsControlPointOutput, 3> i, 
    float3 bary : SV_DomainLocation)
{
    DsOutput o = (DsOutput)0;

    float3 f3Position = 
        bary.x * i[0].f3Position + 
        bary.y * i[1].f3Position +
        bary.z * i[2].f3Position;

    float3 f3Normal = normalize(
        bary.x * i[0].f3Normal +
        bary.y * i[1].f3Normal + 
        bary.z * i[2].f3Normal);

    o.f2TexCoord = 
        bary.x * i[0].f2TexCoord + 
        bary.y * i[1].f2TexCoord + 
        bary.z * i[2].f2TexCoord;

    uddBendNormal(f3Position.x, f3Normal, _Radius, _Width);
    uddBendVertex(f3Position, _Radius, _Width, _Thickness);

    float disp = length(_DispTex.SampleLevel(sampler_DispTex, o.f2TexCoord, 0)) * _DispFactor;
    f3Position.xyz += f3Normal * disp;

    o.f4Position = UnityObjectToClipPos(float4(f3Position.xyz, 1.0));
        
    return o;
}


fixed4 frag(DsOutput i) : SV_Target
{
    return uddGetScreenTexture(i.f2TexCoord) * _Color * _ColorScale;
}

ENDCG

Pass
{
    CGPROGRAM
    #pragma vertex vert
    #pragma fragment frag
    #pragma hull hull
    #pragma domain domain
    #pragma shader_feature ___ INVERT_X
    #pragma shader_feature ___ INVERT_Y
    #pragma shader_feature _FORWARD_Y _FORWARD_Z
    #pragma shader_feature ___ USE_GAMMA_TO_LINEAR_SPACE
    #pragma multi_compile ___ ROTATE90 ROTATE180 ROTATE270
    #pragma multi_compile ___ USE_CLIP
    #pragma multi_compile ___ BEND_ON
    ENDCG
}

}

Fallback "Unlit/Texture"

}