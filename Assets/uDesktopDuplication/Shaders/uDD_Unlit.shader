Shader "uDesktopDuplication/Unlit"
{

Properties
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Texture", 2D) = "white" {}
    _CursorTex ("Cursor Texture", 2D) = "white" {}
    [KeywordEnum(Off, Y, Z)] _Bend("Bending", Int) = 0
    [PowerSlider(10.0)] _Radius("Radius", Range(1, 100)) = 30
    [KeywordEnum(Off, Front, Back)] _Cull("Culling", Int) = 2
}

SubShader
{

Tags { "RenderType"="Opaque" }

Cull [_Cull]

CGINCLUDE

#include "./uDD_Common.cginc"

half _Radius;
half _Width;

v2f vert(appdata v)
{
    v2f o;
    uddBendVertex(v.vertex, _Radius, _Width);
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
    #pragma multi_compile ___ USE_CLIP
    #pragma multi_compile _BEND_OFF _BEND_Y _BEND_Z
    ENDCG
}

}

Fallback "Unlit/Texture"

}