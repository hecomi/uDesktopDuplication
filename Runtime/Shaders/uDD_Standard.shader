Shader "uDesktopDuplication/Standard" 
{

Properties 
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Albedo (RGB)", 2D) = "white" {}
    _Glossiness ("Smoothness", Range(0, 1)) = 0.5
    _Metallic ("Metallic", Range(0, 1)) = 0.0
    _Emissive ("Emissive", Range(0, 10)) = 0.0
    [KeywordEnum(Y, Z)] _Forward("Mesh Forward Direction", Int) = 0
    [Toggle(BEND_ON)] _Bend("Use Bend", Int) = 0
    [PowerSlider(10.0)] _Radius("Bend Radius", Range(1, 100)) = 30
    [PowerSlider(10.0)] _Thickness("Thickness", Range(0.01, 10)) = 1
    _Width("Width", Range(0.0, 10.0)) = 1.0
    [KeywordEnum(Off, Front, Back)] _Cull("Culling", Int) = 2
}

SubShader 
{
    Tags { "RenderType"="Opaque" }

    Cull [_Cull]
    
    CGPROGRAM

    #pragma target 3.0
    #pragma surface surf Standard fullforwardshadows vertex:vert
    #pragma shader_feature ___ INVERT_X
    #pragma shader_feature ___ INVERT_Y
    #pragma shader_feature _FORWARD_Y _FORWARD_Z
    #pragma shader_feature ___ USE_GAMMA_TO_LINEAR_SPACE
    #pragma multi_compile ___ ROTATE90 ROTATE180 ROTATE270
    #pragma multi_compile ___ USE_CLIP
    #pragma multi_compile ___ BEND_ON

    #define SURFACE_SHADER
    #include "./uDD_Common.cginc"

    half _Glossiness;
    half _Metallic;
    half _Emissive;

    void vert(inout appdata_full v)
    {
        uddBendNormal(v.vertex.x, v.normal, _Radius, _Width);
        uddBendVertex(v.vertex.xyz, _Radius, _Width, _Thickness);
    }

    void surf(Input IN, inout SurfaceOutputStandard o) 
    {
        fixed4 c = uddGetScreenTexture(IN.uv_MainTex) * _Color;
        o.Albedo = c.rgb;
        o.Metallic = _Metallic;
        o.Smoothness = _Glossiness;
        o.Alpha = c.a;
        o.Emission = c.rgb * _Emissive;
    }

    ENDCG
}

FallBack "Diffuse"

}
