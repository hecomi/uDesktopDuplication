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
#ifdef USE_GAMMA_TO_LINEAR_SPACE
    if (!IsGammaSpace()) {
        rgb = GammaToLinearSpace(rgb);
    }
#endif
}

inline fixed4 uddGetTexture(sampler2D tex, float2 uv)
{
    uv = uddInvertUV(uv);
#ifdef USE_CLIP
    uv = uddClipUV(uv);
#endif
    fixed4 c = tex2D(tex, uddRotateUV(uv));
    uddConvertToLinearIfNeeded(c.rgb);
    return c;
}

inline fixed4 uddGetScreenTexture(float2 uv)
{
    return uddGetTexture(_MainTex, uv);
}

inline void uddBendVertex(inout float3 v, half radius, half width, half thickness)
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
    v.z *= thickness;
    #elif _FORWARD_Y
    v.y *= thickness;
    #endif
#endif
}

inline float3 uddRotateY(float3 n, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return float3(c * n.x - s * n.z, n.y, s * n.x + c * n.z);
}

inline float3 uddRotateX(float3 n, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return float3(n.x, c * n.y + s * n.z, -s * n.y + c * n.z);
}

inline void uddBendNormal(float4 x, inout float3 n, half radius, half width)
{
#ifdef BEND_ON
    half angle = width * x / radius;
    #ifdef _FORWARD_Z
    n = uddRotateY(n, -angle);
    #elif _FORWARD_Y
    n = uddRotateX(n, -angle);
    #endif
#endif
}

#endif