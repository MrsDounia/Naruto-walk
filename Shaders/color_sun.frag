#version 130
precision mediump float; //Medium precision for float. highp and smallp can also be used
//We still use varying because OpenGLES 2.0 (OpenGL Embedded System, for example for smartphones) does not accept "in" and "out"

uniform vec3 uLightPosition;
uniform vec3 uLightColor;
uniform vec3 uColorMtl;
uniform vec4 uConstantes; //ka, kd, ks, alpha
uniform vec3 uCameraPos;


varying vec3 vary_normal;
varying vec3 vary_world_position;

void main()
{
    //Illumination 
    vec3 N = vary_normal;
    vec3 L = normalize(uLightPosition- vary_world_position);
    vec3 R = normalize(reflect(-L,N));  
    vec3 V = normalize(uCameraPos - vary_world_position); 
    
    vec3 ambiant = uConstantes[0]*uColorMtl*uLightColor;
    vec3 diffuse = uConstantes[1]*max(0.0,dot(N,L))*uColorMtl*uLightColor;
    vec3 specular = uConstantes[2]*pow(max(0.0, dot(R,V)),uConstantes[3])*uLightColor;
    gl_FragColor = vec4(ambiant + diffuse + specular,1.0); 
}