Shader "uDesktopDuplication/Unlit"
{

Properties
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _ColorScale ("ColorScale", Range(0.0, 10.0)) = 1.0
    _MainTex ("Texture", 2D) = "white" {}
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

CGINCLUDE

#include "./uDD_Common.cginc"

v2f vert(appdata v)
{
    v2f o;
    UNITY_SETUP_INSTANCE_ID(v);
    UNITY_INITIALIZE_OUTPUT(v2f, o);
    UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
    uddBendVertex(v.vertex.xyz, _Radius, _Width, _Thickness);
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.uv = TRANSFORM_TEX(v.uv, _MainTex);
    return o;
}

fixed4 frag(v2f i) : SV_Target
{
    return uddGetScreenTexture(i.uv) * _Color * _ColorScale;
}

ENDCG

Pass
{
    CGPROGRAM
    #pragma vertex vert
    #pragma fragment frag
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