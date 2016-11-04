Shader "uDesktopDuplication/Standard" 
{

Properties 
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Albedo (RGB)", 2D) = "white" {}
    _Glossiness ("Smoothness", Range(0, 1)) = 0.5
    _Metallic ("Metallic", Range(0, 1)) = 0.0
    [KeywordEnum(Off, Front, Back)] _Cull("Culling", Int) = 2
}

SubShader 
{
    Tags { "RenderType"="Opaque" }

    Cull [_Cull]
    
    CGPROGRAM

    #pragma target 3.0
    #pragma surface surf Standard fullforwardshadows
    #pragma multi_compile ___ INVERT_X
    #pragma multi_compile ___ INVERT_Y
    #pragma multi_compile ___ VERTICAL
    #pragma multi_compile ___ ROTATE90 ROTATE180 ROTATE270

	#define SURFACE_SHADER
    #include "./uDD_Common.cginc"
	#include "./uDD_Params.cginc"

	half _Glossiness;
	half _Metallic;

    void surf(Input IN, inout SurfaceOutputStandard o) 
    {
        fixed4 c = uddGetScreenTextureWithCursor(_MainTex, _CursorTex, IN.uv_MainTex, _CursorPositionScale) * _Color;
        o.Albedo = c.rgb;
        o.Metallic = _Metallic;
        o.Smoothness = _Glossiness;
        o.Alpha = c.a;
    }

    ENDCG
}

FallBack "Diffuse"

}
