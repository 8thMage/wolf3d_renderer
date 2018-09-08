#version 330
uniform vec2 screen_dim;
layout(location = 0) in vec2 position;

void main()
{
	float screen_ratio=screen_dim.y/screen_dim.x;
	gl_Position.xy= position;
	gl_Position.x*=screen_ratio;
}

