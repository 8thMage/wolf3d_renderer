#version 330
struct MBall
{
	vec2 position;
	float radius;
};

uniform in MBall MBalls[10];
flat in int len;
out vec4 outputColor;
void main()
{
	float value=0;
	vec2 position=gl_FragCoord.xy;
	for(int i=0;i<len;i++)
	{
		vec2 rel_pos=position-MBalls[i].position;
		float rel_dist_sqrd=dot(rel_pos,rel_pos);
		value+=1/sqrt(rel_dist_sqrd)-1/MBalls[i].radius;
	}
	if(value>0)
		outputColor=vec4(0,0,1,1);
	else
		outputColor=vec4(0,1,0,1);
}

