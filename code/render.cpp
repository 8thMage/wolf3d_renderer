struct Render_context;
typedef HGLRC WGLGETCONTEXT(HDC,HGLRC, int*); 
static PFNGLCREATESHADERPROC glCreateShader;

static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLGETSHADERIVPROC glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLGETPROGRAMIVPROC glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
static PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
static PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLBUFFERSUBDATAPROC glBufferSubData;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
static PFNGLUNIFORM3FPROC glUniform3f;
static PFNGLUNIFORM4FPROC glUniform4f;
static PFNGLUNIFORM3IPROC glUniform3i;
static PFNGLUNIFORM3FVPROC glUniform3fv;
static PFNGLUNIFORM2FVPROC glUniform2fv;
static PFNGLUNIFORM1FVPROC glUniform1fv;
static PFNGLUNIFORM3IVPROC glUniform3iv;
static PFNGLUNIFORM2FPROC glUniform2f;
static PFNGLUNIFORM1FPROC glUniform1f;
static PFNGLUNIFORM1IPROC glUniform1i;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
static PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
static PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
static PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;


GLuint CreateShader(GLenum eShaderType, const char* shader_file,int shader_length )
{
	GLuint shader=glCreateShader(eShaderType);
	//int compiled_shader_length;
	glShaderSource(shader,1 , &shader_file, &shader_length);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	Assert(status==GL_TRUE);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		delete[] strInfoLog;

	}
	return shader;
}
GLuint CreateProgram(GLuint* shaders,int count)
{
	GLuint program=glCreateProgram();
	for(int i=0;i<count;i++)
	{
		glAttachShader(program,shaders[i]);
	}
	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	Assert(status==GL_TRUE);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		delete[] strInfoLog;
	}
	return program;
}
GLenum error;

GLuint MBalls_program;
GLuint tri_program;
GLuint positionBufferObject;
void render(MemoryBuffer* queue)
{
	
	Vec2 screen_dim=get_dimensions(window_rect)	;
	glViewport(0,0,(int)screen_dim.x,(int)screen_dim.y);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.7,0.7,0.7,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	//glEnable(GL_DEPTH_TEST);
	//glClearDepth(10000);
	//glClear(GL_DEPTH_BUFFER_BIT);
	if(!MBalls_program)
	{
		char* fs;
		int fs_length;
		char* vs;
		int vs_length;
		GLuint shader_handles[2];
		read_file_alloc((u8*)"../code/MBalls.vs",&vs_length,(void**)&vs);
		shader_handles[0]=CreateShader(GL_VERTEX_SHADER,vs,vs_length);
		read_file_alloc((u8*)"../code/MBalls.fs",&fs_length,(void**)&fs);
		shader_handles[1]=CreateShader(GL_FRAGMENT_SHADER,fs,fs_length);
		VirtualFree(fs,0,MEM_RELEASE);
		VirtualFree(vs,0,MEM_RELEASE);
		GLuint theProgram= CreateProgram(shader_handles,2);
		MBalls_program=(umo)theProgram;
		glUseProgram(theProgram);
		error = glGetError();
		Assert(!error);
	}
	if(!tri_program)
	{
		char* fs;
		int fs_length;
		char* vs;
		int vs_length;
		GLuint shader_handles[2];
		read_file_alloc((u8*)"../code/tri.vs",&vs_length,(void**)&vs);
		shader_handles[0]=CreateShader(GL_VERTEX_SHADER,vs,vs_length);
		read_file_alloc((u8*)"../code/tri.fs",&fs_length,(void**)&fs);
		shader_handles[1]=CreateShader(GL_FRAGMENT_SHADER,fs,fs_length);
		VirtualFree(fs,0,MEM_RELEASE);
		VirtualFree(vs,0,MEM_RELEASE);
		GLuint theProgram= CreateProgram(shader_handles,2);
		tri_program=(umo)theProgram;
		glUseProgram(theProgram);
		error = glGetError();
		Assert(!error);
	}
	Matrix4 matrix={};

	matrix.array[0] = 1.f;
	matrix.array[5] = 1.f;
	matrix.array[10] = -1;
	matrix.array[11] = -1;
	matrix.array[14] = -1;

	Matrix4 projection_transform_matrix=matrix;

	umo current_read_location=queue->original_start;
	while(current_read_location!=queue->place)
	{
		Render_tag* tag=(Render_tag*)current_read_location;
		switch(*tag)
		{
			case RT_MBalls:
			{
				current_read_location+=sizeof(RC_MBalls);
				RC_MBalls* mballs=(RC_MBalls*)tag;
				glUseProgram(MBalls_program);
				int len=mballs->len;
				GLuint len_pos=glGetUniformLocation(MBalls_program,"len");
				glUniform1i(len_pos,mballs->len);
				//for(int i=0;i<len;i++)
				//{
				char* radius_str="MBalls_rad[0]";
				char* position_str="MBalls_pos[0]";
				error = glGetError();
				GLuint radius_pos=glGetUniformLocation(MBalls_program,radius_str);
				GLuint position_pos=glGetUniformLocation(MBalls_program,position_str);
				glUniform1fv(radius_pos,len,mballs->mballs_radius);
				error = glGetError();
				glUniform2fv(position_pos,len,mballs->mballs_position[0].E);
				error = glGetError();

				//}
				float vertex_data[]=
				{
					-1,-1,
					4,-1,
					-1,4
				};
				if(!positionBufferObject)
				{
					glGenBuffers(1,&positionBufferObject);
					glBindBuffer(GL_ARRAY_BUFFER,positionBufferObject);
					glBufferData(GL_ARRAY_BUFFER,sizeof(vertex_data), vertex_data,GL_STATIC_READ);
				}
				glBindBuffer(GL_ARRAY_BUFFER,positionBufferObject);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0,2,GL_FLOAT, GL_FALSE, 0,0);
				glDrawArrays(GL_TRIANGLES,0,3);

				glDisableVertexAttribArray(0);
				glUseProgram(0);
				break;
			}
			case RT_Triangle:
			{
				current_read_location+=sizeof(RC_Triangle);
				RC_Triangle* tri=(RC_Triangle*) tag;
				glUseProgram(tri_program);
				GLuint tri_color=glGetUniformLocation(tri_program,"color");
				glUniform4f(tri_color,1,1,1,1);
				//glBegin(GL_TRIANGLES);
				//glVertex2fv(tri->vrts[1].E);
				//glVertex2fv(tri->vrts[0].E);
				//glVertex2fv(tri->vrts[2].E);
				GLuint a;
				glGenBuffers(1,&a);
				glBindBuffer(GL_ARRAY_BUFFER,a);
				glBufferData(GL_ARRAY_BUFFER,sizeof(tri->vrts[0])*3, tri->vrts[0].E,GL_STATIC_READ);
				//glBindBuffer(GL_ARRAY_BUFFER,a);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0,2,GL_FLOAT, GL_FALSE, 0,0);
				glDrawArrays(GL_TRIANGLES,0,3);

				glDisableVertexAttribArray(0);
				error = glGetError();

				//glEnd();
				break;
			}
		}
	}
	queue->place=queue->original_start;
}

