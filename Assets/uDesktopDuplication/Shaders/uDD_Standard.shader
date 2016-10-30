Shader "uDesktopDuplication/Standard" 
{

Properties 
{
    _Color ("Color", Color) = (1, 1, 1, 1)
    _MainTex ("Albedo (RGB)", 2D) = "white" {}
    _Glossiness ("Smoothness", Range(0, 1)) = 0.5
    _Metallic ("Metallic", Range(0, 1)) = 0.0
}

SubShader 
{
    Tags { "RenderType"="Opaque" }
    
    CGPROGRAM

    #pragma target 3.0
    #pragma surface surf Standard fullforwardshadows
    #pragma multi_compile ___ INVERT_X
    #pragma multi_compile ___ INVERT_Y
    #pragma multi_compile ___ VERTICAL

    #include "./uDD_Common.cginc"

    sampler2D _MainTex;

    struct Input 
    {
        float2 uv_MainTex;
    };

    half _Glossiness;
    half _Metallic;
    fixed4 _Color;

    void surf(Input IN, inout SurfaceOutputStandard o) 
    {
        fixed4 c = uddGetTexture(_MainTex, IN.uv_MainTex) * _Color;
        o.Albedo = c.rgb;
        o.Metallic = _Metallic;
        o.Smoothness = _Glossiness;
        o.Alpha = c.a;
    }

    ENDCG
}

FallBack "Diffuse"

}
