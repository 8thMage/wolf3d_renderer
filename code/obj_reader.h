#ifdef READ_FILE_IMPL
static bool read_file_alloc(u8* path,s32* length,void** file)
{
	bool res=false;
	HANDLE file_handle=CreateFile((char*)path,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(file_handle!=INVALID_HANDLE_VALUE)
	{
		s32 size= GetFileSize(file_handle,0);
		void* buffer=VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
		res=(ReadFile(file_handle,buffer,size,(LPDWORD)length,0 )!=0);
		CloseHandle(file_handle);
		*file=buffer;  
		return res;
	}
	return false;
}
#endif

char* obj_line_header_str_arr[]={
		"",
		"#",
		"vn",
		"vt",
		"v",
		"f",
		"s",
		"g",
		"usemtl",
		"mtllib",
};
int obj_line_header_len_arr[]={
		1,
		1,
		2,
		2,
		1,
		1,
		1,
		1,
		6,
		6,
};
enum obj_line_header
{
	obj_line_header_null=0,
	obj_line_header_comment,
	obj_line_header_vn,
	obj_line_header_vt,
	obj_line_header_v,
	obj_line_header_f,
	obj_line_header_s,
	obj_line_header_g,
	obj_line_header_usemtl,
	obj_line_header_mtllib,
};
struct Face_v
{
	int v_index;
	int n_index;
	int uv_index;
};
struct Obj_file
{
	Extensible_buffer vertices;

	Extensible_buffer normals;
	Extensible_buffer uvs;
	Extensible_buffer faces_v;
	Extensible_buffer faces_length;
};

static void run_until_new_line(u8** file,s32* length)
{
	while(**file!='\n'&&**file!='\r')
	{
		(*file)++;
		(*length)--;
	}
	while(**file=='\n'||**file=='\r')
	{
		(*file)++;
		(*length)--;
	}
}
static void run_until_space_or_new_line(u8** file,s32* length)
{
	while(*length>0&&**file!=' '&&**file!='\n'&&**file!='\r')
	{
		(*file)++;
		(*length)--;
	}
	while(*length>0&&(**file==' '||**file=='\n'||**file=='\r'))
	{
		(*file)++;
		(*length)--;
	}
}
static Obj_file parse_obj_file(void* file,s32 length)
{
	void* curr_line=file;
	Obj_file res;
	res.vertices=new_extensible_buffer(1024);
	res.normals=new_extensible_buffer(1024);
	res.uvs=new_extensible_buffer(1024);
	res.faces_v=new_extensible_buffer(1024);
	res.faces_length=new_extensible_buffer(1024);
	while(length>0)
	{
		obj_line_header line_header=(obj_line_header)0;
		for(int i=0;i<ArrayLength(obj_line_header_str_arr);i++)
		{
			if(es_strcmp_second_non_zero_ended((u8*)obj_line_header_str_arr[i],(u8*)curr_line, obj_line_header_len_arr[i]))
			{
				line_header=(obj_line_header)i;
				break;
			}
		}
		if(line_header!=obj_line_header_v&&
		   line_header!=obj_line_header_vt)
		{
			int x=0;
		}
		switch(line_header)
		{
			case obj_line_header_v:
				{
					Vec3 new_vertex;
					int num= _snscanf((char*)curr_line,(umo)length, "v %f %f %f",&new_vertex.x,&new_vertex.y,&new_vertex.z);
					Assert(num==3);
					Vec3* v=push_struct(&res.vertices,Vec3);
					*v=new_vertex;
					break;
				}
			case obj_line_header_vn:
				{
					Vec3 new_normal;
					int num= _snscanf((char*)curr_line,(umo)length, "vn %f %f %f",&new_normal.x,&new_normal.y,&new_normal.z);
					Assert(num==3);
					Vec3* vn=push_struct(&res.normals,Vec3);
					*vn=new_normal;
					break;
				}
			case obj_line_header_vt:
				{
					Vec3 new_uv;
					int num= _snscanf((char*)curr_line,(umo)length, "vt %f %f %f",&new_uv.x,&new_uv.y,&new_uv.z);
					Assert(num==3||num==2);
					Vec3* uv=push_struct(&res.uvs,Vec3);
					*uv=new_uv;
					break;
				}
			case obj_line_header_f:
				{
					//face_v new_face_v;
					void* original_curr_line=curr_line;
					s32 original_length=length;

					run_until_space_or_new_line((u8**)&curr_line, &length);
					s32* face_length=push_struct(&res.faces_length,s32);
					*face_length=0;
					
					do 
					{
						int v_index;
						int vn_index;
						int vt_index;

						int num= _snscanf((char*)curr_line,(umo)length, "%d/%d/%d",&v_index,&vt_index,&vn_index);
						if(num!=3)
						{
							num= _snscanf((char*)curr_line,(umo)length, "%d//%d",&v_index,&vn_index);
							vt_index=0;
							Assert(num==2);
						}
						if(v_index<0)
						{
							int v_length=(int)((res.vertices.place-res.vertices.original_start)/sizeof(Vec3));
							v_index=v_length+v_index;
						}
						if(vn_index<0)
						{
							int vn_length=(int)((res.normals.place-res.normals.original_start)/sizeof(Vec3));
							vn_index=vn_length+vn_index;
						}
						if(vt_index<0)
						{
							int vt_length=(int)((res.uvs.place-res.uvs.original_start)/sizeof(Vec3));
							vt_index=vt_length+vt_index;
						}
						Face_v* fv=push_struct(&res.faces_v,Face_v);
						fv->v_index=v_index;
						fv->n_index=vn_index;
						fv->uv_index=vt_index;
						
						run_until_space_or_new_line((u8**)&curr_line, &length);
						(*face_length)++;
					}while(length>0&&(*(u8*)curr_line=='-'||*(u8*)curr_line>='0'&&*(u8*)curr_line<='9'));

					curr_line=original_curr_line;
					length=original_length;
					break;
				}
			default:
				{
					break;
				}

		}
		run_until_new_line((u8**)&curr_line, &length);
	}
	return res;
}
