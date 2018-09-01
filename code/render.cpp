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
static PFNGLUNIFORM3IPROC glUniform3i;
static PFNGLUNIFORM3FVPROC glUniform3fv;
static PFNGLUNIFORM3IVPROC glUniform3iv;
static PFNGLUNIFORM2FPROC glUniform2f;
static PFNGLUNIFORM1FPROC glUniform1f;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
static PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
static PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
static PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;


static void render_rectangle(Rect rect,Vec4 color)
{
	glBegin(GL_TRIANGLES);
	glColor4fv(color.xyzw);

	Vec2 lower_left=vec2f(rect.start.x,rect.end.y);
	Vec2 upper_left=vec2f(rect.start.x,rect.start.y);
	Vec2 upper_right=vec2f(rect.end.x,rect.start.y);
	Vec2 lower_right=vec2f(rect.end.x,rect.end.y);
	glVertex2fv(upper_left.E);
	glVertex2fv(upper_right.E);
	glVertex2fv(lower_right.E);

	glVertex2fv(upper_left.E);
	glVertex2fv(lower_right.E);
	glVertex2fv(lower_left.E);
	glEnd();
}
static void render_triangle_3d(Vec3 tri[3],Vec4 color)
{
	glBegin(GL_TRIANGLES);
	glColor4fv(color.xyzw);

	glVertex3fv(tri[0].xyz);
	glVertex3fv(tri[1].xyz);
	glVertex3fv(tri[2].xyz);

	glEnd();
}
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
static GLint offsetLocation =0;
GLuint projectionMatrix=0;
GLuint Transform_location=0;
GLenum error;

const float vertex_data[] = {
	1,  1, 1, 1.0f,
	1, 0, 1, 1.0f,
	0,  1, 1, 1.0f,

	1, 0, 1, 1.0f,
	0, 0, 1, 1.0f,
	0,  1, 1, 1.0f,

	1,  1, 0, 1.0f,
	0,  1, 0, 1.0f,
	1, 0, 0, 1.0f,

	1, 0, 0, 1.0f,
	0,  1, 0, 1.0f,
	0, 0, 0, 1.0f,

	0,  1, 1, 1.0f,
	0, 0, 1, 1.0f,
	0, 0, 0, 1.0f,

	0,  1, 1, 1.0f,
	0, 0, 0, 1.0f,
	0,  1, 0, 1.0f,

	1,  1, 1, 1.0f,
	1, 0, 0, 1.0f,
	1, 0, 1, 1.0f,

	1,  1, 1, 1.0f,
	1,  1, 0, 1.0f,
	1, 0, 0, 1.0f,

	1,  1, 0, 1.0f,
	1,  1, 1, 1.0f,
	0,  1, 1, 1.0f,

	1,  1, 0, 1.0f,
	0,  1, 1, 1.0f,
	0,  1, 0, 1.0f,

	1, 0, 0, 1.0f,
	0, 0, 1, 1.0f,
	1, 0, 1, 1.0f,

	1, 0, 0, 1.0f,
	0, 0, 0, 1.0f,
	0, 0, 1, 1.0f,




	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,

	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,

	0.8f, 0.8f, 0.8f, 1.0f,
	0.8f, 0.8f, 0.8f, 1.0f,
	0.8f, 0.8f, 0.8f, 1.0f,

	0.8f, 0.8f, 0.8f, 1.0f,
	0.8f, 0.8f, 0.8f, 1.0f,
	0.8f, 0.8f, 0.8f, 1.0f,

	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,

	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,

	0.5f, 0.5f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.0f, 1.0f,

	0.5f, 0.5f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.0f, 1.0f,

	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,

	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,

	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,

	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,

};
GLuint positionBufferObject=0;
GLuint vao=0;
GLuint cube_program=0;
GLuint cube_dim_program=0;
void render(MemoryBuffer* queue)
{
	
	Vec2 screen_dim=get_dimensions(window_rect)	;
	glViewport(0,0,(int)screen_dim.x,(int)screen_dim.y);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.7,0.7,0.7,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	glEnable(GL_DEPTH_TEST);
	glClearDepth(10000);
	glClear(GL_DEPTH_BUFFER_BIT);
	if(!cube_dim_program)
	{
		if(!positionBufferObject)
		{
			glGenBuffers(1,&positionBufferObject);
			error = glGetError();
			glBindBuffer(GL_ARRAY_BUFFER,positionBufferObject);
			error = glGetError();
			glBufferData(GL_ARRAY_BUFFER,sizeof(vertex_data), vertex_data,GL_STATIC_READ);
			error = glGetError();
			glBindBuffer(GL_ARRAY_BUFFER,0);

			error = glGetError();
			glGenVertexArrays(1,&vao);
			error = glGetError();
			glBindVertexArray(vao);
			error = glGetError();
		}
		{
			error=glGetError();
			Assert(!error);
			char* fs;
			int fs_length;
			char* vs;
			int vs_length;
			GLuint shader_handles[2];
			read_file_alloc((u8*)"../code/cube_dims.vs",&vs_length,(void**)&vs);
			shader_handles[0]=CreateShader(GL_VERTEX_SHADER,vs,vs_length);
			read_file_alloc((u8*)"../code/cube.fs",&fs_length,(void**)&fs);
			shader_handles[1]=CreateShader(GL_FRAGMENT_SHADER,fs,fs_length);
			VirtualFree(fs,0,MEM_RELEASE);
			VirtualFree(vs,0,MEM_RELEASE);
			GLuint theProgram= CreateProgram(shader_handles,2);
			cube_dim_program=(umo)theProgram;
			glUseProgram(theProgram);
			projectionMatrix= glGetUniformLocation(theProgram, "projectionMatrix");
			//offsetLocation= glGetUniformLocation(theProgram, "chunk_offset");
			Transform_location= glGetUniformLocation(theProgram, "transform");
			error = glGetError();
			Assert(!error);
		}

	}
	Matrix4 matrix={};

//	r32 matrix[16] = {};

	matrix.array[0] = 1.f;
	matrix.array[5] = 1.f;
	matrix.array[10] = -1;
	matrix.array[11] = -1;
	matrix.array[14] = -1;
	glUniformMatrix4fv(projectionMatrix, 1, 1, matrix.array);

	Matrix4 projection_transform_matrix=matrix;

	umo current_read_location=queue->original_start;
	while(current_read_location!=queue->place)
	{
		Render_tag* tag=(Render_tag*)current_read_location;
		switch(*tag)
		{
			case RT_cube_array:
			{
				RC_render_cube_array* cubes=(RC_render_cube_array*)tag;
				current_read_location+=sizeof(*cubes);
				if(!*cubes->VB)
				{
					glGenBuffers(1,(GLuint*)cubes->VB);
					glBindBuffer(GL_ARRAY_BUFFER,*(GLuint*)cubes->VB);
					glBufferData(GL_ARRAY_BUFFER,sizeof(Vec3i)*cubes->length,cubes->poses,GL_STATIC_READ);
					error = glGetError();
					Assert(!error);
				}
				r32 matrix[16] = {};

				matrix[0] = 0.5f;
				matrix[5] = 0.5f;
				matrix[10] = -1;
				matrix[11] = -1;
				matrix[14] = -2;
				glUniformMatrix4fv(projectionMatrix, 1, 1, matrix);

				//error = glGetError();
				glUseProgram(cube_program);
				glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
//				error = glGetError();
				Assert(!error);
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
//				error = glGetError();
				Assert(!error);
				glVertexAttribPointer(0,4,GL_FLOAT, GL_FALSE, 0,0);
				glVertexAttribPointer(1,4,GL_FLOAT, GL_FALSE, 0,(void*)(12*3*4*4) );
//				error = glGetError();
				Assert(!error);
				glBindBuffer(GL_ARRAY_BUFFER, *(GLuint*)cubes->VB);
				glEnableVertexAttribArray(2);
				glVertexAttribIPointer(2,3,GL_INT,0,0);
				glVertexAttribDivisor(2,1);
//				error = glGetError();
				Assert(!error);

				glDrawArraysInstanced(GL_TRIANGLES,0,12*3,cubes->length);
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
			
			break;
			}
		case RT_cube_dim_array:
			{
				RC_render_cube_dim_array* cubes=(RC_render_cube_dim_array*)tag;
				current_read_location+=sizeof(*cubes);
				if(!*cubes->VB)
				{
					glGenBuffers(1,(GLuint*)cubes->VB);
					glBindBuffer(GL_ARRAY_BUFFER,*(GLuint*)cubes->VB);
					glBufferData(GL_ARRAY_BUFFER,sizeof(Vec3i)*2*cubes->length,cubes->poses_dims_f,GL_STATIC_READ);
					error = glGetError();
					Assert(!error);
				}

				cubes->start_bounding_box.z*=-1;
				cubes->end_bounding_box.z*=-1;
				Vec4 bb_transformed[8]={};
				for(int i=0;i<8;i++)
				{
					Vec3 bb={};
					if(i&1)
					{
						bb.x=cubes->end_bounding_box.x;
					}
					else
					{
						bb.x=cubes->start_bounding_box.x;
					}
					
					if(i&2)
					{
						bb.y=cubes->end_bounding_box.y;
					}
					else
					{
						bb.y=cubes->start_bounding_box.y;
					}
					if(i&4)
					{
						bb.z=cubes->end_bounding_box.z;
					}
					else
					{
						bb.z=cubes->start_bounding_box.z;
					}
					bb_transformed[i]=transform(projection_transform_matrix,bb);
				}
				bool cull=true;
				for(int i=0;i<8&&cull;i++)
				{
					if(bb_transformed[i].w>0) cull=false;
				}
				if(cull) break;
				cull =true;
				for(int i=0;i<8&&cull;i++)
				{
					if(bb_transformed[i].w>bb_transformed[i].x) cull=false;
				}
				if(cull) break;

				cull =true;
				for(int i=0;i<8&&cull;i++)
				{
					if(bb_transformed[i].w>bb_transformed[i].y) cull=false;
				}
				if(cull) break;

				cull =true;
				for(int i=0;i<8&&cull;i++)
				{
					if(bb_transformed[i].w>-bb_transformed[i].x) cull=false;
				}
				if(cull) break;

				cull =true;
				for(int i=0;i<8&&cull;i++)
				{
					if(bb_transformed[i].w>-bb_transformed[i].y) cull=false;
				}
				if(cull) break;

				//OutputDebugString("painted");
					glUseProgram(cube_dim_program);
				//error = glGetError();
				glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
				error = glGetError();
				Assert(!error);
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				error = glGetError();
				Assert(!error);
				glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 4*sizeof(float),0);
				glVertexAttribPointer(1,4,GL_FLOAT, GL_FALSE, 0,(void*)(12*3*4*4) );
				error = glGetError();
				Assert(!error);
				glBindBuffer(GL_ARRAY_BUFFER, *(GLuint*)cubes->VB);
				glEnableVertexAttribArray(2);
				glEnableVertexAttribArray(3);
			//	glVertexAttribIPointer(2,2,GL_UNSIGNED_BYTE,4,0);
			//	glVertexAttribIPointer(2,2,GL_UNSIGNED_BYTE,4,0);
				glVertexAttribIPointer(2,3,GL_INT,2*sizeof(Vec3i),0);
				glVertexAttribIPointer(3,3,GL_INT,2*sizeof(Vec3i),(void*)(sizeof(Vec3i)));
				error = glGetError();
				Assert(!error);
				glVertexAttribDivisor(2,1);
				glVertexAttribDivisor(3,1);
				//glUniform3iv(offsetLocation,1,cubes->position.xyz);
				error = glGetError();

				Assert(!error);
				glDrawArraysInstanced(GL_TRIANGLES,0,12*3,cubes->length);
				error = glGetError();
				Assert(!error);
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
				glDisableVertexAttribArray(3);

				break;
			}

		case RT_transform:
			{
				RC_transform* transform=(RC_transform*)tag;
				current_read_location+=sizeof(*transform);
				glUniformMatrix4x3fv(Transform_location, 1, 0, transform->transform.array);
				projection_transform_matrix=composition(matrix,transform->transform);
				break;
			}
	}
	}
	queue->place=queue->original_start;
}

