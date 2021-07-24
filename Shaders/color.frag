#version 130
precision mediump float; //Medium precision for float. highp and smallp can also be used
//We still use varying because OpenGLES 2.0 (OpenGL Embedded System, for example for smartphones) does not accept "in" and "out"

uniform sampler2D uTexture; 


varying vec2 vary_uv; //The UV coordinate, going from (0.0, 0.0) to (1.0, 1.0)

void main()
{
    vec4 color = texture(uTexture, vary_uv); //Apply the texture to this pixel //couleur qui change oar rapport à la texture
    gl_FragColor = color;

}