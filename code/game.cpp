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
void push_triangle_to_render_queue(MemoryBuffer* render_queue, Vec2 vrt1, Vec2 vrt2,Vec2 vrt3)
{
	RC_Triangle* rc_tri=push_struct(render_queue,RC_Triangle);
	rc_tri->tag=RT_Triangle;
	rc_tri->vrts[0]=vrt1;
	rc_tri->vrts[1]=vrt2;
	rc_tri->vrts[2]=vrt3;
}
Vec2 grad_mballs(MBalls mbs,Vec2 mouse_pos)
{
	Vec2 res={};
	forei(mbs.len)
	{
		Vec2 rel_pos=mouse_pos-mbs.pos[i];
		float rel_dist_sqrd=DotProduct(rel_pos,rel_pos);
		float recp_rel_dist=1/sqrtf(rel_dist_sqrd);
		Vec2 adder = rel_pos*recp_rel_dist*recp_rel_dist*recp_rel_dist;
		res-=adder;
	}
	res = Normalize(res);
	return res;
}
void go_game(Input* input, GameMemory* game_memory, read_file_type* read_file)
{
	Game_data* game_data=game_memory->game_data;
	if(!game_data)
	{
		game_memory->game_data=push_struct(game_memory->const_buffer,Game_data);
		game_data=game_memory->game_data;
		game_data->mbs.len=2;
		game_data->mbs.rad=push_array(game_memory->const_buffer,float,game_data->mbs.len);
		game_data->mbs.pos=push_array(game_memory->const_buffer,Vec2,game_data->mbs.len);
		game_data->mbs.pos[0]={1500,1500};
		game_data->mbs.rad[0]=300;
		game_data->mbs.pos[1]={1500,1000};
		game_data->mbs.rad[1]=300;
		//game_data->mbs_pos[2]={1000,1000};
		//game_data->mbs_rad[2]=300;
		game_memory->render_queue=new_memory_buffer(game_memory->const_buffer,10 MB);
	}
	push_mballs_to_render_queue(game_memory->render_queue,game_data->mbs.pos,game_data->mbs.rad,game_data->mbs.len);
	Vec2 mouse_pos=input->mouse_pos;
	mouse_pos.y=game_memory->draw_context.screen->height-mouse_pos.y;
	Vec2 grad=grad_mballs(game_data->mbs,mouse_pos)*100;
	Vec2 grad_perp=Perp(grad);
	push_triangle_to_render_queue(game_memory->render_queue,mouse_pos+grad,mouse_pos+grad_perp,mouse_pos-grad_perp);
}
