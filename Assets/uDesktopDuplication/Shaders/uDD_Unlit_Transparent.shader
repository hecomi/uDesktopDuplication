Shader "uDesktopDuplication/Unlit Transparent"
{

Properties
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Texture", 2D) = "white" {}
    [KeywordEnum(Off, Front, Back)] _Cull("Culling", Int) = 2
}

SubShader
{

Tags { "Queue"="Transparent" "IgnoreProjector"="True" "RenderType"="Transparent" }

Cull [_Cull]
ZWrite On
Blend SrcAlpha OneMinusSrcAlpha

CGINCLUDE

#include "./uDD_Common.cginc"

v2f vert(appdata v)
{
    v2f o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.uv = TRANSFORM_TEX(v.uv, _MainTex);
    return o;
}

fixed4 frag(v2f i) : SV_Target
{
    return fixed4(uddGetScreenTextureWithCursor(i.uv).rgb, 1.0) * _Color;
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

Fallback "Unlit/Transparent"

}
