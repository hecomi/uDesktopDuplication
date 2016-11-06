#ifndef UDD_PARAMS_CGINC
#define UDD_PARAMS_CGINC

sampler2D _MainTex;
fixed4 _Color;
sampler2D _CursorTex;

half4 _CursorPositionScale;
#define _CursorX _CursorPositionScale.x
#define _CursorY _CursorPositionScale.y
#define _CursorWidth _CursorPositionScale.z
#define _CursorHeight _CursorPositionScale.w

half4 _ClipPositionScale;
#define _ClipX _ClipPositionScale.x
#define _ClipY _ClipPositionScale.y
#define _ClipWidth _ClipPositionScale.z
#define _ClipHeight _ClipPositionScale.w

#ifndef SURFACE_SHADER
float4 _MainTex_ST;
#endif

#endif