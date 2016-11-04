#ifndef UDD_PARAMS_CGINC
#define UDD_PARAMS_CGINC

sampler2D _MainTex;
fixed4 _Color;
sampler2D _CursorTex;
half4 _CursorPositionScale;

#ifndef SURFACE_SHADER
float4 _MainTex_ST;
#endif

#endif