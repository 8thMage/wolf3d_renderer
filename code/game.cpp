#include "game.h"
#include "zlib.cpp"
#include "render.h"
enum special_names
{
	sn_not_special=0,
	sn_Y,
	sn_Blocks,
	sn_Add
};
special_names nbt_skip_string_info(void** region,bool named)
{
	special_names res=sn_not_special;
	if(named)
	{
		umo length=read_n_bytes_BE(region,2);
		if(length==1&&es_strcmp_second_non_zero_ended("Y",(u8*)(*region),(int)length))
		{
			res=sn_Y;
		}
		else if(length==6&&es_strcmp_second_non_zero_ended("Blocks",(u8*)(*region),(int)length))
		{
			res=sn_Blocks;
		}
		else if(length==3&&es_strcmp_second_non_zero_ended("Add",(u8*)(*region),(int)length))
		{
			res=sn_Add;
		}
		*(u8**)region+=length;
}
	return res;
}
int size_LUT[]={0,1,2,4,8,4,8,1,1,0,0,4,8};
int length_LUT[]={0,0,0,0,0,0,0,4,2,4,0,4,4};
		
void parse_region_file(void** region, MC_chunk* chunk, u32 single_type, bool named,int x_chunk,int z_chunk)
{
	short block_ids[4096]={}; 
	int y_chunk=0;
	do
	{
		u32 type;
		if (!single_type) 
			type=read_byte(region);
		else
			type=single_type;
		int size=size_LUT[type];
		int prefix_size=length_LUT[type];
		if(type==0)
		{
			break ;
		}
		else if(type==9)//list 
		{
			nbt_skip_string_info(region,named);	
			u32 new_type=(u32)read_n_bytes_BE(region,1);
			int arr_length=(int)read_n_bytes_BE(region,4);
			for(int i=0;i<arr_length;i++)
			{
				parse_region_file(region,chunk,new_type,false,x_chunk,z_chunk);
			}
		}
		else if(type==10)
		{
			nbt_skip_string_info(region,named);	
			parse_region_file(region,chunk,0,true,x_chunk,z_chunk);
		}
		else
		{
			special_names sn=nbt_skip_string_info(region,named);	
			if(sn==sn_Y)
			{
				y_chunk=(int)read_n_bytes_BE(region,size);
			}
			else if(sn==sn_Blocks)
			{
				int arr_length=1;
				if(prefix_size)
				{
					arr_length=(int)read_n_bytes_BE(region,prefix_size);
				}
				Assert(arr_length==4096);
				for(int i=0;i<4096;i++)
				{
					umo block_id=read_n_bytes_BE(region,size);
					block_ids[i]+=(short)block_id;
				}
			}
			else if(sn==sn_Add)
			{
				int arr_length=1;
				if(prefix_size)
				{
					arr_length=(int)read_n_bytes_BE(region,prefix_size);
				}
				Assert(arr_length==4096);
				for(int i=0;i<4096;i++)
				{
					umo block_add=read_n_bytes_BE(region,size);
					block_ids[i]+=(((short)block_add)<<8);
				}
			}
			else
			{

				umo arr_length=1;
				if(prefix_size)
				{
					arr_length=read_n_bytes_BE(region,prefix_size);
				}
				*(u8**)region+=size*arr_length;
			}
		}


	}while(!single_type);
	for(int i=0;i<4096;i++)
	{
		short is_not_air=block_ids[i];
		if(is_not_air)
		{
//			if(i/16%16==0)
			{
				int x=i%16+x_chunk;
				int z=(i/16)%16+z_chunk;
				int y=(i/16)/16+y_chunk*16;
				Vec3i pos=vec3i(x,y,z);
				chunk->poses[chunk->current_write_pos++]= pos;
				chunk->start_bounding_box.x=Min(pos.x-1,chunk->start_bounding_box.x);
				chunk->start_bounding_box.y=Min(pos.y-1,chunk->start_bounding_box.y);
				chunk->start_bounding_box.z=Min(pos.z-1 ,chunk->start_bounding_box.z);
				chunk->end_bounding_box.x=Max(pos.x + 1,chunk->end_bounding_box.x);
				chunk->end_bounding_box.y=Max(pos.y + 1,chunk->end_bounding_box.y);
				chunk->end_bounding_box.z=Max(pos.z +1,chunk->end_bounding_box.z);
			}
		}
	}
}
void turn_chunks_to_optimised_chunks(MC_chunk* chunk,MC_chunk_optimised* opt_chunk)
{
	int current_max_y[16][16] = {};
	int current_min_y[16][16] = {};
	opt_chunk->start_bounding_box=chunk->start_bounding_box;
	opt_chunk->end_bounding_box=chunk->end_bounding_box;
	for(int z=0;z<16;z++)
	{
		for(int x=0;x<16;x++)
		{
			current_max_y[z][x]=-2;
			current_min_y[z][x]=-2;
		}
	}
	forei(chunk->current_write_pos)
	{
		Vec3i pos=chunk->poses[i];
		u8 rel_z_pos=(u8)(pos.z%16);
		u8 rel_x_pos=(u8)(pos.x%16);
		int* curr_max_y=&current_max_y[rel_z_pos][rel_x_pos];
		int* curr_min_y=&current_min_y[rel_z_pos][rel_x_pos];
		if(*curr_max_y!=pos.y-1)
		{
			if (*curr_max_y != -2)
			{
				//opt_chunk->poses_dims[opt_chunk->current_write_pos*4] = vec3i(pos.x, *curr_min_y, pos.z);
				
				opt_chunk->poses_dims_f[opt_chunk->current_write_pos*2]=vec3i(rel_x_pos+chunk->position.x,*curr_min_y,rel_z_pos+chunk->position.z);
				opt_chunk->poses_dims_f[opt_chunk->current_write_pos++*2 +1]=vec3i(1,*curr_max_y-*curr_min_y+1,1);
			}
			*curr_min_y = pos.y;
		}
		*curr_max_y=pos.y;
	}
	for(int rel_z=0;rel_z<16;rel_z++)
	{

		for(int rel_x=0;rel_x<16;rel_x++)
		{
				
			int* curr_max_y=&current_max_y[rel_z][rel_x];
			int* curr_min_y=&current_min_y[rel_z][rel_x];
			if (*curr_max_y != -2)
			{
				
				opt_chunk->poses_dims_f[opt_chunk->current_write_pos*2]=vec3i(rel_x+chunk->position.x,*curr_min_y,rel_z+chunk->position.z);
				opt_chunk->poses_dims_f[opt_chunk->current_write_pos++*2 +1]=vec3i(1,*curr_max_y-*curr_min_y+1,1);
			}
		}
	}
}
void turn_optimised_chunks_to_more_optimised_chunks(MC_chunk_optimised* opt_chunk)
{
	for(int i=0;i<opt_chunk->current_write_pos;i++)
	{
		for(int j=0;j<opt_chunk->current_write_pos;j++)
		{
			if(i!=j)
			{
				Vec3i* pos_dim_1=opt_chunk->poses_dims_f+2*i;
				Vec3i* pos_dim_2=opt_chunk->poses_dims_f+2*j;
				if(pos_dim_1[0].x+pos_dim_1[1].x==pos_dim_2[0].x&&pos_dim_1[0].z==pos_dim_2[0].z)
				{
					if(pos_dim_1[0].y>=pos_dim_2[0].y&&
							pos_dim_1[0].y+pos_dim_1[1].y<=pos_dim_2[0].y+pos_dim_2[1].y)
					{

						if(pos_dim_1[0].y+pos_dim_1[1].y<pos_dim_2[0].y+pos_dim_2[1].y)
						{
							opt_chunk->poses_dims_f[opt_chunk->current_write_pos*2 ]=vec3i(pos_dim_2[0].x,pos_dim_1[0].y+pos_dim_1[1].y,pos_dim_2[0].z);
							opt_chunk->poses_dims_f[opt_chunk->current_write_pos++*2 +1 ]=vec3i(pos_dim_2[1].x,pos_dim_2[1].y+pos_dim_2[0].y-(pos_dim_1[0].y+pos_dim_1[1].y),pos_dim_2[1].z);
						}
						pos_dim_2[1].y = pos_dim_1[0].y - pos_dim_2[0].y;
						pos_dim_1[1].x += pos_dim_2[1].x;
					}
				}
			}
		}
	}
	Vec3i* write_pos_f=opt_chunk->poses_dims_f;
	u8* write_pos=opt_chunk->poses_dims;
	for(int i=0;i<opt_chunk->current_write_pos;i++)
	{
		Vec3i* pos_dim_1=opt_chunk->poses_dims_f+2*i;
		if(pos_dim_1[1].x!=0&&pos_dim_1[1].y!=0&&pos_dim_1[1].z!=0)
		{
			*write_pos_f=*pos_dim_1;
			write_pos_f[1]=pos_dim_1[1];
			u8 rel_x_pos=(u8)(pos_dim_1[0].x%16);
			u8 rel_z_pos=(u8)(pos_dim_1[0].z%16);
			write_pos[0]=(u8)(rel_x_pos|(rel_z_pos<<4));
			write_pos[1]=(u8)(pos_dim_1[0].y);

			rel_x_pos=(u8)(pos_dim_1[1].x%16);
			rel_z_pos=(u8)(pos_dim_1[1].z%16);
			write_pos[2]=(u8)(rel_x_pos|(rel_z_pos<<4u));
			write_pos[3]=(u8)(pos_dim_1[1].y);

			write_pos_f+=2;
			write_pos+=4;
		}
	}
	opt_chunk->current_write_pos=(int)(write_pos_f-opt_chunk->poses_dims_f)/2;

}

void push_cubes_to_render(MemoryBuffer* render_queue,MC_chunk* chunk)
{
	RC_render_cube_array* rc=push_struct(render_queue,RC_render_cube_array);
	rc->tag=RT_cube_array;
	rc->length=chunk->current_write_pos;
	rc->poses=chunk->poses;
	rc->VB = &chunk->VB;
}
void push_cubes_to_render(MemoryBuffer* render_queue,MC_chunk_optimised* opt_chunk)
{
	RC_render_cube_dim_array* rc=push_struct(render_queue,RC_render_cube_dim_array);
	rc->tag=RT_cube_dim_array;
	rc->length=opt_chunk->current_write_pos;
	rc->poses_dims=opt_chunk->poses_dims;
	rc->position=opt_chunk->position;
	rc->poses_dims_f=opt_chunk->poses_dims_f;
	rc->start_bounding_box=opt_chunk->start_bounding_box;
	rc->end_bounding_box=opt_chunk->end_bounding_box;
	rc->VB = &opt_chunk->VB;
}
void push_transform_to_render(MemoryBuffer* render_queue,Matrix4x3 transform)
{
	RC_transform* rc=push_struct(render_queue,RC_transform);
	rc->tag=RT_transform;
	rc->transform = transform;
}
void go_game(Input* input, GameMemory* game_memory, read_file_type* read_file)
{
	int zlib_file_length=0;
	void* zlib_file_buffer=0;
	static int x=0;
	MC_chunk temp_chunk={};
	MC_chunk_optimised temp_chunk_opt={};
	if(!x)
	{
	Assert(read_file("../Future CITY 4.4/region/r.0.0.mca",game_memory->temp_buffer,&zlib_file_length,&zlib_file_buffer));
		MemoryBuffer start_temp_buffer=*game_memory->temp_buffer;
		int length=0;
//		int j=0;
		for(int j=0;j<32;j++)
//		int i=10;
		for(int i=0;i<32;i++)

//		for(int k=0;k<2;k++)
		{
			temp_chunk = {};

			temp_chunk.start_bounding_box=vec3f(maximum_r32,maximum_r32,maximum_r32);
			temp_chunk.end_bounding_box=vec3f(minimum_r32,minimum_r32,minimum_r32);
			temp_chunk_opt = {};
			temp_chunk.poses=push_array(game_memory->temp_buffer,Vec3i,4096*16);
			temp_chunk_opt.poses_dims_f=push_array(game_memory->temp_buffer,Vec3i,2*4096*16);
			temp_chunk_opt.poses_dims=push_array(game_memory->temp_buffer,u8,4*4096*16);
			void* decompressed_data=read_and_parse_region_buffer(zlib_file_buffer,zlib_file_length,i,j,game_memory->temp_buffer);
			if(decompressed_data!=0)
			{
				parse_region_file(&decompressed_data,&temp_chunk,0,true,(i)*16,j*16);
			}
			temp_chunk.position=vec3i(i*16,0,j*16);
			temp_chunk_opt.position = vec3i(i * 16, 0, j * 16);
			turn_chunks_to_optimised_chunks(&temp_chunk,&temp_chunk_opt);
			turn_optimised_chunks_to_more_optimised_chunks(&temp_chunk_opt);
			game_memory->opt_chunk[j*32+i]=temp_chunk_opt;
			game_memory->opt_chunk[j*32+i].poses_dims_f=push_array(game_memory->const_buffer,Vec3i,2*temp_chunk_opt.current_write_pos);
			game_memory->opt_chunk[j*32+i].poses_dims=push_array(game_memory->const_buffer,u8,4*temp_chunk_opt.current_write_pos);
			{
				Vec3i* dest=game_memory->opt_chunk[j*32+i].poses_dims_f;
				Vec3i* src=temp_chunk_opt.poses_dims_f;
				es_memcopy(dest,src,temp_chunk_opt.current_write_pos*2);
			}
			{
				u8* dest=game_memory->opt_chunk[j*32+i].poses_dims;
				u8* src=temp_chunk_opt.poses_dims;
				es_memcopy(dest,src,temp_chunk_opt.current_write_pos*4);
			}
			length+=temp_chunk_opt.current_write_pos;
			*game_memory->temp_buffer=start_temp_buffer;
		}
/*		game_memory->opt_chunk_combined.current_write_pos=length;
		game_memory->opt_chunk_combined.poses_dims=push_array(game_memory->const_buffer,Vec3i,2*length);
		int current_location=0;
		for(int i=0;i<32;i++)
		{
			for(int j=0;j<32;j++)
			{
				Vec3i* src=game_memory->opt_chunk[j*32+i].poses_dims;
				Vec3i* dest=game_memory->opt_chunk_combined.poses_dims+current_location*2;
				int length=game_memory->opt_chunk[j*32+i].current_write_pos;
				es_memcopy(dest,src,length*2);
				current_location+=length;
			}
		}*/
	
		game_memory->render_queue=new_memory_buffer(game_memory->temp_buffer,200*1024);//this_does_not_work
		game_memory->draw_context.transform=translate(-16*20,-85,-16*2);
		x=1;
	}

	if(input->button_w.state)
	{
		game_memory->draw_context.transform=composition(translate(0,0,0.4),game_memory->draw_context.transform);
	}
	if(input->button_s.state)
	{
		game_memory->draw_context.transform=composition(translate(0,0,-0.4),game_memory->draw_context.transform);
	}
	if(input->button_e.state)
	{
		game_memory->draw_context.transform=composition(translate(-0.4,0,0),game_memory->draw_context.transform);
	}
	if(input->button_q.state)
	{
		game_memory->draw_context.transform=composition(translate(0.4,0,0),game_memory->draw_context.transform);
	}	
	if(input->button_r.state)
	{
		game_memory->draw_context.transform=composition(translate(0,-0.4,0),game_memory->draw_context.transform);
	}
	if(input->button_f.state)
	{
		game_memory->draw_context.transform=composition(translate(0,0.4,0),game_memory->draw_context.transform);
	}
	if(input->button_a.state)
	{
		game_memory->draw_context.transform=composition(rotate_y(-0.05),game_memory->draw_context.transform);
	}
	if(input->button_d.state)
	{
		game_memory->draw_context.transform=composition(rotate_y(0.05),game_memory->draw_context.transform);
	}
	push_transform_to_render(game_memory->render_queue,game_memory->draw_context.transform);
//	push_cubes_to_render(game_memory->render_queue,&game_memory->opt_chunk_combined);
	for(int i=0;i<ArrayLength(game_memory->opt_chunk);i++)
//	int i=20;
	{
		push_cubes_to_render(game_memory->render_queue,&game_memory->opt_chunk[i]);
	}
}
