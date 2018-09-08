#version 330

uniform float radius;
uniform vec2 center;
uniform vec4 color;
uniform vec2 screen_dim;
out vec4 outputColor;
void main()
{
	vec2 position=(gl_FragCoord.xy*2-screen_dim)/screen_dim.y;
	vec2 center_uniform=center;
	//center_uniform.x*=screen_dim.y/screen_dim.x;
	vec2 rel_pos=position-center_uniform;
	float dist2=dot(rel_pos,rel_pos);
	if(dist2<radius*radius)
	{
		outputColor=color;
	}
}

