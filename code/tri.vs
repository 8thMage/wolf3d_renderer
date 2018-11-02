#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normals;
uniform mat4 matrix;
uniform float time;
out vec4 color;
void main()
{
	float x=position.x*cos(time/100)-position.z*sin(time/100);
	float z=position.x*sin(time/100)+position.z*cos(time/100);
	float nz=normals.x*sin(time/100)+normals.z*cos(time/100);
	gl_Position=matrix*vec4(x,position.y,z,1);
	//gl_Position.w=position.z+1;
	//gl_Position.x*=screen_ratio;
	float color_value=nz+0.1f;
//	float color_value=0;
	color=vec4(color_value,color_value,color_value,1);
//	color=vec4(0,0,0,1);
}

