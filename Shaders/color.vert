#version 130
precision mediump float;

uniform mat4 uMVP;
uniform mat3 uInvModelMatrix;
uniform mat4 uModelMatrix;

attribute vec3 vPosition; //Depending who compiles, these variables are not "attribute" but "in". In this version (130) both are accepted. in should be used later
attribute vec3 vNormal;
attribute vec2 vUV;

varying vec3 vary_normal;
varying vec3 vary_world_position;
varying vec2 vary_uv;

void main()
{
    
    gl_Position = uMVP * vec4(vPosition, 1.0); //We need to put vPosition as a vec4. Because vPosition is a vec3, we need one more value (w) which is here 1.0. Hence x and y go from -w to w hence -1 to +1. Premultiply this variable if you want to transform the position.
    vary_uv=vUV;

}