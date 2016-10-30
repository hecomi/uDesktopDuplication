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
}

inline fixed4 uddGetTexture(sampler2D tex, float2 uv)
{
    uddInvertUV(uv);
    fixed4 c = tex2D(tex, uv);
    if (!IsGammaSpace()) {
        c.rgb = GammaToLinearSpace(c.rgb);
    }
    return c;
}

#endif