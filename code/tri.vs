#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normals;
uniform mat4 matrix;
out vec4 color;
void main()
{
	gl_Position.xyz= position;
	gl_Position.w= 1;
	gl_Position=matrix*gl_Position;
	//gl_Position.x*=screen_ratio;
	float color_value=normals.z+0.1f;
	color=vec4(color_value,color_value,color_value,1);
	color=vec4(0,0,0,1);
}

