#ifndef UDD_COMMON_CGINC
#define UDD_COMMON_CGINC

#include "UnityCG.cginc"
#include "./uDD_Params.cginc"

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

inline float2 uddInvertUV(float2 uv)
{
#ifdef INVERT_X
    uv.x = 1.0 - uv.x;
#endif
#ifdef INVERT_Y
    uv.y = 1.0 - uv.y;
#endif
    return uv;
}

inline float2 uddRotateUV(float2 uv)
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

inline float2 uddClipUV(float2 uv)
{
    uv.x = _ClipX + uv.x * _ClipWidth;
    uv.y = _ClipY + uv.y * _ClipHeight;
    return uv;
}

inline void uddConvertToLinearIfNeeded(inout fixed3 rgb)
{
    if (!IsGammaSpace()) {
        rgb = GammaToLinearSpace(rgb);
    }
}

inline fixed4 uddGetScreenTexture(float2 uv)
{
    uv = uddInvertUV(uv);
#ifdef USE_CLIP
    uv = uddClipUV(uv);
#endif
    fixed4 c = tex2D(_MainTex, uddRotateUV(uv));
    uddConvertToLinearIfNeeded(c.rgb);
    return c;
}

inline void uddBendVertex(inout float4 v, half radius, half width, half thickness)
{
#ifdef BEND_ON
    half angle = width * v.x / radius;
    #ifdef _FORWARD_Z
    v.z *= thickness;
    radius += v.z;
    v.z -= radius * (1 - cos(angle));
    #elif _FORWARD_Y
    v.y *= thickness;
    radius += v.y;
    v.y += radius * (1 - cos(angle));
    #endif
    v.x = radius * sin(angle) / width;
#else
    #ifdef _FORWARD_Z
    v.y *= thickness;
    #elif _FORWARD_Y
    v.z *= thickness;
    #endif
#endif
}

#endif