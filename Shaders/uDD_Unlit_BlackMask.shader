Shader "uDesktopDuplication/Unlit BlackMask"
{

Properties
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Texture", 2D) = "white" {}
    _Mask ("Mask", Range(0, 1)) = 0.1
    [KeywordEnum(Off, Front, Back)] _Cull("Culling", Int) = 2
}

SubShader
{

Tags { "Queue"="Transparent" "IgnoreProjector"="True" "RenderType"="Transparent" }

Cull [_Cull]
ZWrite On
Blend SrcAlpha OneMinusSrcAlpha

CGINCLUDE

#include "UnityCG.cginc"
#include "./uDD_Common.cginc"

struct appdata
{
    float4 vertex : POSITION;
    float2 uv     : TEXCOORD0;
};

struct v2f
{
    float4 vertex : SV_POSITION;
    float2 uv     : TEXCOORD0;
};

sampler2D _MainTex;
float4 _MainTex_ST;
fixed4 _Color;
fixed _Mask;

v2f vert(appdata v)
{
    v2f o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.uv = TRANSFORM_TEX(v.uv, _MainTex);
    return o;
}

fixed4 frag(v2f i) : SV_Target
{
    fixed4 tex = uddGetTexture(_MainTex, i.uv);
    fixed alpha = pow((tex.r + tex.g + tex.b) / 3.0, _Mask);
    return fixed4(tex.rgb * _Color.rgb, alpha);
}

ENDCG

Pass
{
    CGPROGRAM
    #pragma vertex vert
    #pragma fragment frag
    #pragma multi_compile ___ INVERT_X
    #pragma multi_compile ___ INVERT_Y
    #pragma multi_compile ___ VERTICAL
    ENDCG
}

}

Fallback "Unlit/Texture"

}
