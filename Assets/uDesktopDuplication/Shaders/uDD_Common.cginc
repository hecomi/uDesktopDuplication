#ifndef UDD_COMMON_CGINC
#define UDD_COMMON_CGINC

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

struct Input 
{
	float2 uv_MainTex;
};

float2 uddInvertUV(float2 uv)
{
#ifdef INVERT_X
	uv.x = 1.0 - uv.x;
#endif
#ifdef INVERT_Y
	uv.y = 1.0 - uv.y;
#endif
	return uv;
}

float2 uddRotateUV(float2 uv)
{
#ifdef ROTATE90
    float2 tmp = uv;
    uv.x = tmp.y;
    uv.y = 1.0 - tmp.x;
#elif ROTATE180
	uv.x = 1.0 - uv.x;
	uv.y = 1.0 - uv.y;
#elif ROTATE270
    float2 tmp = uv;
    uv.x = 1.0 - tmp.y;
    uv.y = tmp.x;
#endif
	return uv;
}

inline void uddConvertToLinearIfNeeded(inout fixed3 rgb)
{
    if (!IsGammaSpace()) {
        rgb = GammaToLinearSpace(rgb);
    }
}

inline fixed4 uddGetScreenTexture(sampler2D tex, float2 uv)
{
    fixed4 c = tex2D(tex, uv);
    return c;
}

inline fixed4 uddGetCursorTexture(sampler2D tex, float2 uv, fixed4 cursorPosScale)
{
    uv.x = (uv.x - cursorPosScale.x) / cursorPosScale.z;
    uv.y = (uv.y - cursorPosScale.y) / cursorPosScale.w;
    fixed4 c = tex2D(tex, uv);
	fixed a = c.a * step(0, uv.x) * step(0, uv.y) * step(uv.x, 1) * step(uv.y, 1);
	c *= step(0.01, a);
	return c;
}

inline fixed4 uddGetScreenTextureWithCursor(sampler2D screenTex, sampler2D cursorTex, float2 uv, fixed4 cursorPosScale)
{
	uv = uddInvertUV(uv);
	fixed4 screen = uddGetScreenTexture(screenTex, uddRotateUV(uv));
	fixed4 cursor = uddGetCursorTexture(cursorTex, uv, cursorPosScale);
	fixed4 color = lerp(screen, cursor, cursor.a);
	uddConvertToLinearIfNeeded(color.rgb);
	return color;
}

#endif