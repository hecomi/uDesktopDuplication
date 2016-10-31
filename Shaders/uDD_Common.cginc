#ifndef UDD_COMMON_CGINC
#define UDD_COMMON_CGINC

#include "UnityCG.cginc"

inline void uddInvertUV(inout float2 uv)
{
#ifdef INVERT_X
    uv.x = 1.0 - uv.x;
#endif
#ifdef INVERT_Y
    uv.y = 1.0 - uv.y;
#endif
#ifdef VERTICAL
    float2 tmp = uv;
    uv.x = tmp.y;
    uv.y = 1.0 - tmp.x;
#endif
}

inline void uddToLinearIfNeeded(inout float3 rgb)
{
    if (!IsGammaSpace()) {
        rgb = GammaToLinearSpace(rgb);
    }
}

inline fixed4 uddGetTexture(sampler2D tex, float2 uv)
{
    uddInvertUV(uv);
    fixed4 c = tex2D(tex, uv);
    uddToLinearIfNeeded(c.rgb);
    return c;
}

#endif