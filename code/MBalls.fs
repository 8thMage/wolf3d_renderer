#version 330


uniform float MBalls_rad[10];
uniform vec2 MBalls_pos[10];
uniform int len;
out vec4 outputColor;
void main()
{
	float value=0;
	vec2 position=gl_FragCoord.xy;
	for(int i=0;i<len;i++)
	{
		vec2 rel_pos=position-MBalls_pos[i];
		float rel_dist_sqrd=dot(rel_pos,rel_pos);
		value+=1/sqrt(rel_dist_sqrd)-1/MBalls_rad[i];
	}
	if(value>0)
		outputColor=vec4(0,0,1,1);
	else
		outputColor=vec4(0,1,0,1);
}

