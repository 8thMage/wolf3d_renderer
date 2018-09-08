#include "game.h"
#include "render.h"
Vec2 screen_dim_;
void push_mballs_to_render_queue(MemoryBuffer* render_queue, MBalls* mbs)
{
	RC_MBalls* rc_mbs=push_struct(render_queue,RC_MBalls);
	rc_mbs->tag=RT_MBalls;
	rc_mbs->mballs_position=mbs->pos;
	rc_mbs->mballs_radius=mbs->rad;
	rc_mbs->len=mbs->len;
}
void push_balls_to_render_queue(MemoryBuffer* render_queue, Balls* bs)
{
	RC_Balls* rc_bs=push_struct(render_queue,RC_Balls);
	rc_bs->tag=RT_Balls;
	rc_bs->pos=bs->pos;
	rc_bs->len=bs->len;
	rc_bs->radius=bs->radius;
}
void push_dust_to_render_queue(MemoryBuffer* render_queue, Balls* bs)
{
	RC_Dust* rc_dust=push_struct(render_queue,RC_Dust);
	rc_dust->tag=RT_Dust;
	rc_dust->pos=bs->pos;
	rc_dust->len=bs->len;
}
void push_triangle_to_render_queue(MemoryBuffer* render_queue, Vec2 vrt1, Vec2 vrt2,Vec2 vrt3)
{
	RC_Triangle* rc_tri=push_struct(render_queue,RC_Triangle);
	rc_tri->tag=RT_Triangle;
	rc_tri->vrts[0]=vrt1;
	rc_tri->vrts[1]=vrt2;
	rc_tri->vrts[2]=vrt3;
}
Vec2 grad_mballs(MBalls* mbs,Vec2 mouse_pos)
{
	Vec2 res={};
	forei(mbs->len)
	{
		Vec2 rel_pos=mouse_pos-mbs->pos[i];
		float rel_dist_sqrd=DotProduct(rel_pos,rel_pos);
		float recp_rel_dist=1/sqrtf(rel_dist_sqrd);
		Vec2 adder = rel_pos*recp_rel_dist*recp_rel_dist*recp_rel_dist;
		res-=adder*mbs->rad[i];
	}
	return res;
}
float mballs_fun(MBalls* mbs,Vec2 pos)
{
	float res=-1;
	forei(mbs->len)
	{
		Vec2 rel_pos=pos-mbs->pos[i];
		float rel_dist_sqrd=rel_pos.x*rel_pos.x+rel_pos.y*rel_pos.y;
		//float rel_dist_sqrd=DotProduct(rel_pos,rel_pos);
		__m128 rel_dist_sqrd_vec=_mm_set_ss(rel_dist_sqrd);
		float recp_rel_dist=_mm_cvtss_f32(_mm_rsqrt_ss(rel_dist_sqrd_vec));
		res+=recp_rel_dist*mbs->rad[i];
	}
	return res;
}
Vec2 screen_pos_to_logical_pos(Vec2 input,Vec2 screen_dim)//used for mouse
{
	Vec2 ret;
	float screen_ratio=screen_dim.x/screen_dim.y;
	ret=HaddamardProduct(input,vec2f(2.f/screen_dim.x,2.f/screen_dim.y));
	ret-=vec2f(1,1);
	ret.y=-ret.y;//from -1 to 1 
	ret.x*=screen_ratio;//from -16/9 to 16/9 and -1 to 1
	return ret;
}
Vec2 logical_coord_to_uniform_coord(Vec2 input,Vec2 screen_dim)//used for mouse
{
	float screen_ratio=screen_dim.x/screen_dim.y;
	Vec2 ret;
	ret=input;
	ret.x/=screen_ratio;
	return ret;
}

//bool balls_collided(Balls* balls,int i,MBalls* mbs,MemoryBuffer* render_queue,Vec2* collision)
bool balls_collided(Balls* balls,int i,MBalls* mbs,Vec2* collision)
{
	bool early_out=false;
	bool collided=false;
	forej(mbs->len)
	{
		if(Norm2(balls->pos[i]-mbs->pos[i])<balls->radius*balls->radius)
		{
			early_out=true;
			break;
		}
	}
	if(early_out) return false;
	Vec2 iter=balls->pos[i];
	float previus_f=-1000;
	while(true)
	{
		float f=mballs_fun(mbs,iter);
		if(fabs(f-previus_f)<1E-4)
		{
			break;
		}
		Vec2 grad=Normalize(grad_mballs(mbs,iter));
		iter-=grad*0.1*balls->radius;
		iter=balls->pos[i]-balls->radius*Normalize(balls->pos[i]-iter);
		//debugging this
		Vec2 grad_perp=Perp(grad);
		Vec2 uniform_grad=logical_coord_to_uniform_coord(grad,screen_dim_)*0.01;
		Vec2 uniform_perp=logical_coord_to_uniform_coord(grad_perp,screen_dim_)*0.01;
		Vec2 uniform_iter=logical_coord_to_uniform_coord(iter,screen_dim_);
		//push_triangle_to_render_queue(render_queue,uniform_iter+uniform_grad,uniform_iter+uniform_perp,uniform_iter-uniform_perp);

		previus_f=f;
	}
	if(mballs_fun(mbs,iter)<0)
	{
		*collision=iter;
		return true;
	}

	return false;
}
void dust_logic(Balls* balls,MBalls* mbs)
{
	Vec2 collision={};
	for(int k=0;k<10;k++)
	{
		for(int i=0;i<balls->len;i++)
		{
			balls->pos[i]+=balls->velocity[i]/10;
			if(mballs_fun(mbs,balls->pos[i])<0)
			{
				Vec2 grad=Normalize(grad_mballs(mbs,balls->pos[i]));
				balls->velocity[i]-=2*grad*DotProduct(grad,balls->velocity[i]);
			}
		}
	}
}
void balls_logic(Balls* balls,MBalls* mbs)
{
	Vec2 collision={};
	int iterations=100;
	for(int k=0;k<iterations;k++)
	{
//#pragma omp parallel for
		for(int i=0;i<balls->len;i++)
		{
			balls->pos[i]+=balls->velocity[i]/(float)iterations;
			if(balls_collided(balls,i,mbs,&collision))
			{
				Vec2 grad=Normalize(grad_mballs(mbs,collision));
				float dot_prod=DotProduct(grad,balls->velocity[i]);
				if(dot_prod<0)
					balls->velocity[i]-=2*grad*dot_prod;
			}
		}
	}
}
void go_game(Input* input, GameMemory* game_memory, read_file_type* read_file)
{
	Game_data* game_data=game_memory->game_data;
	screen_dim_=vec2f((float)game_memory->draw_context.screen->width,(float)game_memory->draw_context.screen->height);
	Balls* balls;
	if(!game_data&&input->button_p.state)
	{
		game_memory->game_data=push_struct(game_memory->const_buffer,Game_data);
	
		game_data=game_memory->game_data;
		game_data->mbs.len=5;
		game_data->mbs.rad=push_array(game_memory->const_buffer,float,game_data->mbs.len);
		game_data->mbs.pos=push_array(game_memory->const_buffer,Vec2,game_data->mbs.len);
		game_data->mbs.pos[1]={0,0.5};
		game_data->mbs.rad[1]=0.1;
		game_data->mbs.pos[0]={0,-0.5};
		game_data->mbs.rad[0]=0.2;
		game_data->mbs.pos[2]={0.5,0.5};
		game_data->mbs.rad[2]=0.2;
		game_data->mbs.pos[3]={-0.8,-0.5};
		game_data->mbs.rad[3]=0.1;
		game_data->mbs.pos[4]={-0.8,0};
		game_data->mbs.rad[4]=0.05;
		game_memory->render_queue=new_memory_buffer(game_memory->const_buffer,10 MB);

		balls=&game_data->balls;
		balls->len=100;
		balls->pos=push_array(game_memory->const_buffer,Vec2,balls->len);
		balls->velocity=push_array(game_memory->const_buffer,Vec2,balls->len);
		for(int i=0;i<balls->len;i++)
		{
			balls->pos[i]=vec2f(0,i/1000.f-0.5000f);
			balls->velocity[i]=vec2f(0.01,0);
//			balls->velocity[i]=vec2f(0.0,0);
			//balls->pos[i]=vec2f(-0.5,-0.5);
		}
		balls->radius=0.03;
	}
	if(game_data)
	{
		balls=&game_data->balls;

		push_mballs_to_render_queue(game_memory->render_queue,&game_data->mbs);

		Vec2 mouse_pos=input->mouse_pos;
		Vec2 logical_mouse_pos=screen_pos_to_logical_pos(mouse_pos,screen_dim_);

		Vec2 grad=Normalize(grad_mballs(&game_data->mbs,logical_mouse_pos))*0.01;

		Vec2 grad_perp=Perp(grad)*10;
		Vec2 uniform_mouse_pos=logical_coord_to_uniform_coord(logical_mouse_pos,screen_dim_);
		Vec2 uniform_grad_perp=logical_coord_to_uniform_coord(grad_perp,screen_dim_);
		Vec2 uniform_grad=logical_coord_to_uniform_coord(grad,screen_dim_);
		//	push_triangle_to_render_queue(game_memory->render_queue,uniform_mouse_pos+uniform_grad*10,uniform_mouse_pos-uniform_grad_perp,uniform_mouse_pos+uniform_grad_perp);

		balls_logic(balls,&game_data->mbs);
//		push_dust_to_render_queue(game_memory->render_queue,balls);

//		balls->pos[0]=logical_mouse_pos;
		push_balls_to_render_queue(game_memory->render_queue,balls);
//		balls_collided(balls,0,&game_data->mbs,game_memory->render_queue,&logical_mouse_pos);
	}
}
