Shader "Diem/AlphaChannel" {
	Properties 
	{
		_MainTex("Main Texture (RGBA)", 2D) = "white" {}
	}


	SubShader 
	{
		Pass 
		{
			Name "AlphaChannel"
			ZTest Always Cull Off ZWrite Off
			Fog { Mode off }

			CGPROGRAM
			#pragma target 3.0
			#pragma vertex vert_img
			#pragma fragment frag
			#pragma fragmentoption ARB_precision_hint_fastest 
			#include "UnityCG.cginc"
						
			uniform sampler2D _MainTex;
 
			float4 frag (v2f_img i) : COLOR
			{
				//return tex2D (_MainTex, i.uv) * tex2D (_MainTex, i.uv).a;
				return tex2D (_MainTex, i.uv).a;
			}
			ENDCG
		}
		
	}

	Fallback Off
} 
