Shader "BackgroundTexture" 
{
   Properties 
   {
      _MainTex ("Texture", 2D) = "white" {}
   }
   SubShader {
      Tags { "Queue" = "Background" } // render before everything else
 
      Pass 
	  {
		 Cull Off 
		 ZWrite Off
 
         CGPROGRAM
 
         #pragma vertex vert  
         #pragma fragment frag 

         #include "UnityCG.cginc" 
 
         uniform sampler2D _MainTex;
 
         struct vertexInput {
            float4 vertex : POSITION;
            float4 texcoord : TEXCOORD0;
         };
         struct vertexOutput {
            float4 vertex : SV_POSITION;
            float4 texcoord : TEXCOORD0;
         };
 
         vertexOutput vert(vertexInput input) 
         {
            vertexOutput output;
 
			output.vertex = float4(
				input.vertex.x * 2.f,
				input.vertex.y * 2.f,
				1, 
				1.0f
			);

			output.texcoord = input.texcoord;

            return output;
         }
 
         float4 frag(vertexOutput input) : COLOR
         {
            return tex2D(_MainTex, input.texcoord.xy);   
         }
 
         ENDCG
      }
   }
}
