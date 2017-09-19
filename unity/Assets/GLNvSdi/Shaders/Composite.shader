// Upgrade NOTE: replaced 'mul(UNITY_MATRIX_MVP,*)' with 'UnityObjectToClipPos(*)'



Shader "GLNvSdi/Composite" 
{
	Properties	
	{	
		_MainTex ("Foreground (RGBA)", 2D) = "" {}	
		_BgTex ("Background (RGB)", 2D) = "" {}	
		_GammaCorrection("Gamma Correction ", Float) = 2.2
		_Scale ("Image Scaling Factor ", Float) = 1.0	
	}
	
	CGINCLUDE
	
	#include "UnityCG.cginc"

	uniform sampler2D	_BgTex;
	uniform sampler2D	_MainTex;
	uniform float		_GammaCorrection;
	uniform float		_Scale;
	
	v2f_img vert( appdata_img v ) 
	{
		v2f_img o;
		o.pos = UnityObjectToClipPos(v.vertex);
		o.pos.xy *= _Scale;
		o.uv = v.texcoord.xy;
		return o;
	}
	
	float4 frag(v2f_img i) : COLOR 
	{
		float4 bg = tex2D(_BgTex, i.uv);
		float4 fg = tex2D(_MainTex, i.uv);
		float alpha = clamp(fg.a, 0, 1);
		fg = float4(pow(fg.rgb, _GammaCorrection), fg.a);
		return (alpha * fg) + ((1.0 - alpha) * bg);
	}

	ENDCG 
	
Subshader 
{
 Pass 
 {
	  ZTest Always Cull Off ZWrite Off
	  Fog { Mode off }      

      CGPROGRAM
      #pragma fragmentoption ARB_precision_hint_fastest 
      #pragma vertex vert
      #pragma fragment frag
      ENDCG
  }
  
}

Fallback off
	
} // shader
