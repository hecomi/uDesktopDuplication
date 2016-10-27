Shader "uDesktopDuplication/Unlit"
{

Properties
{
    _MainTex ("Texture", 2D) = "white" {}
}

SubShader
{

Tags { "RenderType"="Opaque" }

CGINCLUDE

#include "UnityCG.cginc"

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

v2f vert (appdata v)
{
    v2f o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.uv = TRANSFORM_TEX(v.uv, _MainTex);
    return o;
}

fixed4 frag (v2f i) : SV_Target
{
#ifdef INVERT_X
    i.uv.x *= -1.0;
#endif
#ifdef INVERT_Y
    i.uv.y *= -1.0;
#endif
    return tex2D(_MainTex, i.uv);
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

}
