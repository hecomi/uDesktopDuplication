Shader "uDesktopDuplication/Unlit Transparent"
{

Properties
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Texture", 2D) = "white" {}
}

SubShader
{

Tags { "Queue"="Transparent" "IgnoreProjector"="True" "RenderType"="Transparent" }

ZWrite Off
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

v2f vert(appdata v)
{
    v2f o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.uv = TRANSFORM_TEX(v.uv, _MainTex);
    return o;
}

fixed4 frag(v2f i) : SV_Target
{
    uddInvertUV(i.uv);
    return fixed4(tex2D(_MainTex, i.uv).rgb, 1.0) * _Color;
}

ENDCG

Pass
{
    CGPROGRAM
    #pragma vertex vert
    #pragma fragment frag
    #pragma multi_compile ___ INVERT_X
    #pragma multi_compile ___ INVERT_Y
    ENDCG
}

}

Fallback "Unlit/Transparent"

}
