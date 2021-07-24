#version 130
precision mediump float; //Medium precision for float. highp and smallp can also be used

varying vec4 varyColor; //Sometimes we use "out" instead of "varying". "out" should be used in later version of GLSL.

uniform vec3 uLightPosition;
uniform vec3 uLightColor;
uniform vec3 uColorMtl;
uniform vec4 uConstantes; //ka, kd, ks, alpha
//We still use varying because OpenGLES 2.0 (OpenGL Embedded System, for example for smartphones) does not accept "in" and "out"

varying vec2 vary_uv; //The UV coordinate, going from (0.0, 0.0) to (1.0, 1.0)
uniform sampler2D uTexture; //The texture

void main()
{
	vec4 color = texture(uTexture, vary_uv); //Apply the texture to this pixel //couleur qui change oar rapport à la texture
	//Illumination if needed
	gl_FragColor = varyColor;

}
