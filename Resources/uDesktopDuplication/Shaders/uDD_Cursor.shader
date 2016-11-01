Shader "uDesktopDuplication/Cursor"
{

Properties
{
    _MainTex ("Texture", 2D) = "white" {}
}

SubShader
{

Tags { "RenderType"="Opaque" }

ZWrite Off
Offset -1, -1

CGINCLUDE

#include "UnityCG.cginc"
#include "../../../Shaders/uDD_Common.cginc"

half4 _PositionScale;
#define _PointerX      _PositionScale.x
#define _PointerY      _PositionScale.y
#define _PointerWidth  _PositionScale.z
#define _PointerHeight _PositionScale.w

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

v2f vert(appdata v)
{
    v2f o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.uv = v.uv;
    return o;
}

fixed4 frag(v2f i) : SV_Target
{
    uddInvertUV(i.uv);
    i.uv.x = (i.uv.x - _PointerX) / _PointerWidth;
    i.uv.y = (i.uv.y - _PointerY) / _PointerHeight;
    fixed4 color = tex2D(_MainTex, i.uv);
    color.a *= step(0, i.uv.x) * step(0, i.uv.y) * step(i.uv.x, 1) * step(i.uv.y, 1);
    clip(color.a - 0.01);
    uddToLinearIfNeeded(color.rgb);
    return color;
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
