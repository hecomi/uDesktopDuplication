Shader "uDesktopDuplication/Unlit"
{

Properties
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Texture", 2D) = "white" {}
    [Toggle(USE_BEND)] _UseBend("UseBend", Float) = 0
	[PowerSlider(10.0)]_Radius ("Radius", Range(3, 100)) = 10
    [KeywordEnum(Off, Front, Back)] _Cull("Culling", Int) = 2
}

SubShader
{

Tags { "RenderType"="Opaque" }

Cull [_Cull]

CGINCLUDE

#include "./uDD_Common.cginc"
#include "./uDD_Params.cginc"

fixed _Radius;

v2f vert(appdata v)
{
    v2f o;
#ifdef USE_BEND
	half a = v.vertex.x / _Radius;
	v.vertex.x =  _Radius * sin(a);
	v.vertex.y += _Radius * (1 - cos(a));
#endif
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.uv = TRANSFORM_TEX(v.uv, _MainTex);
    return o;
}

fixed4 frag(v2f i) : SV_Target
{
	return uddGetScreenTextureWithCursor(_MainTex, _CursorTex, i.uv, _CursorPositionScale);
}

ENDCG

Pass
{
    CGPROGRAM
    #pragma vertex vert
    #pragma fragment frag
    #pragma shader_feature ___ INVERT_X
    #pragma shader_feature ___ INVERT_Y
    #pragma shader_feature ___ VERTICAL
    #pragma shader_feature ___ USE_BEND
    ENDCG
}

}

Fallback "Unlit/Texture"

}