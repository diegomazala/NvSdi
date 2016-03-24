

Shader "GLNvSdi/SdiComposite" 
{
	Properties	
	{	
		_MainTex ("Foreground (RGBA)", 2D) = "" {}	
		_BgTex ("Background (RGB)", 2D) = "" {}	
		_Scale ("Image Scaling Factor ", Float) = 1.0	
	}
	
	CGINCLUDE
	
	#include "UnityCG.cginc"

	uniform sampler2D	_BgTex;
	uniform sampler2D	_MainTex;
	uniform float		_Scale;
	
	v2f_img vert( appdata_img v ) 
	{
		v2f_img o;
		o.pos = mul(UNITY_MATRIX_MVP, v.vertex);
		o.pos.xy *= _Scale;
		o.uv = v.texcoord.xy;
		return o;
	}
	
	float4 frag(v2f_img i) : COLOR 
	{
		float4 bg = tex2D(_BgTex, i.uv);
		float4 fg = tex2D(_MainTex, i.uv);
		return (fg.a * fg) + ((1.0 - fg.a) * bg);
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
