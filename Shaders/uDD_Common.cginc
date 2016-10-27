#ifndef UDD_COMMON_CGINC
#define UDD_COMMON_CGINC

inline void uddInvertUV(inout float2 uv)
{
#ifdef INVERT_X
    uv.x *= -1.0;
#endif
#ifdef INVERT_Y
    uv.y *= -1.0;
#endif
}

#endif