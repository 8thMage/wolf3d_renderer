#version 330
layout(location = 0) in vec2 position;

void main()
{
	gl_Position.xy= position;
	gl_Position.z= 1;
	gl_Position.w= 1;
}

