Shader "uDesktopDuplication/Unlit"
{

Properties
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Texture", 2D) = "white" {}
    [Toggle(USE_BEND)] _UseBend("UseBend", Float) = 0
    [PowerSlider(10.0)]_Radius ("Radius", Range(10, 100)) = 30
    [KeywordEnum(Off, Front, Back)] _Cull("Culling", Int) = 2
}

SubShader
{

Tags { "RenderType"="Opaque" }

Cull [_Cull]

CGINCLUDE

#include "./uDD_Common.cginc"

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
    return uddGetScreenTextureWithCursor(i.uv);
}

ENDCG

Pass
{
    CGPROGRAM
    #pragma vertex vert
    #pragma fragment frag
    #pragma multi_compile ___ INVERT_X
    #pragma multi_compile ___ INVERT_Y
    #pragma multi_compile ___ ROTATE90 ROTATE180 ROTATE270
    #pragma multi_compile ___ USE_BEND
    #pragma multi_compile ___ USE_CLIP
    ENDCG
}

}

Fallback "Unlit/Texture"

}