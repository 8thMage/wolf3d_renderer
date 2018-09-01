#include "game.h"
#include "render.h"
void push_mballs_to_render_queue(MemoryBuffer* render_queue, Vec2* mbs_pos, float* mbs_rad,int mbs_len)
{
	RC_MBalls* rc_mbs=push_struct(render_queue,RC_MBalls);
	rc_mbs->tag=RT_MBalls;
	rc_mbs->mballs_position=mbs_pos;
	rc_mbs->mballs_radius=mbs_rad;
	rc_mbs->len=mbs_len;
}
void go_game(Input* input, GameMemory* game_memory, read_file_type* read_file)
{
	Game_data* game_data=game_memory->game_data;
	if(!game_data)
	{
		game_memory->game_data=push_struct(game_memory->const_buffer,Game_data);
		game_data=game_memory->game_data;
		game_data->mbs_len=2;
		game_data->mbs_rad=push_array(game_memory->const_buffer,float,game_data->mbs_len);
		game_data->mbs_pos=push_array(game_memory->const_buffer,Vec2,game_data->mbs_len);
		game_data->mbs_pos[0]={1500,1500};
		game_data->mbs_rad[0]=300;
		game_data->mbs_pos[1]={1500,1000};
		game_data->mbs_rad[1]=300;
		//game_data->mbs_pos[2]={1000,1000};
		//game_data->mbs_rad[2]=300;
		game_memory->render_queue=new_memory_buffer(game_memory->const_buffer,10 MB);
	}
	push_mballs_to_render_queue(game_memory->render_queue,game_data->mbs_pos,game_data->mbs_rad,game_data->mbs_len);
	Vec2 mouse_pos=input->mouse_pos;
}
