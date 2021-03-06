#version 330

uniform float radius;
uniform vec2 center;
uniform vec4 color;
out vec4 outputColor;
void main()
{
	vec2 position=(gl_FragCoord.xy*2-screen_dim)/screen_dim.y;
	vec2 rel_pos=position-center;
	vec2 rel_pos2=rel_pos*rel_pos;
	float dist2=rel_pos2.x+rel_pos2.y;
	if(dist2<radius*radius)
	{
		outputColor=color;
	}
}

