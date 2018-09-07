#version 330


uniform float MBalls_rad[10];
uniform vec2 MBalls_pos[10];
uniform int len;
uniform vec2 screen_dim;
out vec4 outputColor;
void main()
{
	float value=-1.;
	vec2 position=gl_FragCoord.xy;
	position.y=(-screen_dim.y+position.y*2)/screen_dim.y;
	position.x=(-screen_dim.x+position.x*2)/screen_dim.y;
	for(int i=0;i<len;i++)
	{
		vec2 rel_pos=position-MBalls_pos[i];
		float rel_dist_sqrd=dot(rel_pos,rel_pos);
		value+=MBalls_rad[i]/sqrt(rel_dist_sqrd);
	}
	if(value>0)
		outputColor=vec4(0,0,1,1);
	else
		outputColor=vec4(0,1,0,1);
	if(value>100)
		outputColor=vec4(1,0,1,1);
}

