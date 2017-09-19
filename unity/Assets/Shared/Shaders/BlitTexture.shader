Shader "Diem/BlitTexture" {
	Properties 
	{
		_MainTex("Input Texture (RGBA)", 2D) = "white" {}
		_GammaCorrection("Gamma Correction", Float) = 2.2
		_FixAspect("Fix Aspect Ratio", Float) = 1.0
	}


	SubShader 
	{
		Pass 
		{
			Name "BlitTexture"
			ZTest Always Cull Off ZWrite Off
			Fog { Mode off }

			CGPROGRAM
			#pragma target 3.0
			#pragma vertex vert_img
			#pragma fragment frag
			#pragma fragmentoption ARB_precision_hint_fastest 
			#include "UnityCG.cginc"
						
			uniform sampler2D _MainTex;
			uniform float _FixAspect;
			uniform float _GammaCorrection;

			//float3 GammaToLinear(float3 rgb)
			//{
			//	return pow(rgb, 2.2f);
			//}

			//float3 LinearToGamma(float3 rgb)
			//{
			//	return pow(rgb, 0.454545f);	// pow(rgb, 1.f/2.2f);
			//}
 
			float4 frag (v2f_img i) : COLOR
			{
				float2 uv_norm = i.uv * 2.0f - float2(1.0f, 1.0f);
				uv_norm.x = uv_norm.x * _FixAspect;
				float2 uv = uv_norm * 0.5f + float2(0.5f, 0.5f);
				float4 color = tex2D(_MainTex, uv);
				float3 rgb = pow(color.rgb, _GammaCorrection);
				return float4(rgb, color.a);
			}
			ENDCG
		}
		
	}

	Fallback Off
} 
