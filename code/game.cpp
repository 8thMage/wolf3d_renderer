#include "game.h"
#include "render.h"
#include "stdio.h"
Vec2 screen_dim_;
MemoryBuffer* render_queue;
int render_current_x;
int render_current_y;
bool was_pressed(ButtonInfo button)
{
	bool res=((button.state==true&&button.changes==1)||(button.changes>1));
	return res;
}
bool was_released(ButtonInfo button)
{
	bool res=((button.state==false&&button.changes==1)||(button.changes>1));

	return res;
}
void draw_rect(PictureBuffer* buffer,int start_x,int start_y,int end_x,int end_y,u32 color)
{
	for(int y=start_y;y<end_y;y++)
	{
		for(int x=start_x;x<end_x;x++)
		{
			int index=x+y*buffer->pitch;
			buffer->picture[index]=color;
		}
	}
}
void draw_rect(PictureBuffer* buffer,Rect rect,u32 color)
{
	rect.startx+=render_current_x;
	rect.endx+=render_current_x;
	rect.starty+=render_current_y;
	rect.endy+=render_current_y;

	RC_rect* rc_rect=push_struct(render_queue,RC_rect);
	rc_rect->tag=RT_rect;
	rc_rect->rect=rect;
	u32 b=color&0xff;
	u32 g=(color>>8)&0xff;
	u32 r=(color>>16)&0xff;
	u32 a=(color>>24)&0xff;
	u32 new_color=(a<<24)|(b<<16)|(g<<8)|r;
	rc_rect->color=new_color;
//	draw_rect(buffer,rect_start_x,rect_start_y,rect_end_x,rect_end_y,color);

}
void render_map(Map* map,Player* player, PictureBuffer* sub_screen)
{
	int height=map->height;
	int width=map->width;
	float pixel_per_tile_x=(float)sub_screen->width/(float)width;
	float pixel_per_tile_y=(float)sub_screen->height/(float)height;
	Vec2 pixel_per_tile=vec2f(pixel_per_tile_x,pixel_per_tile_y);
	for(int y=0;y<height;y++)
	{
		for(int x=0;x<width;x++) 
		{
			Rect rect=make_rect_from_mincorner_width_height(pixel_per_tile_x*x,pixel_per_tile_y*y,pixel_per_tile);
			bool wall=map->arr[x+y*height];
			if(wall)
				draw_rect(sub_screen,rect,0xff00ff00);
			else
				draw_rect(sub_screen,rect,0xff);
		}
	}
	for(int y=0;y<height;y++)
	{
		for(int x=0;x<width;x++) 
		{
			int rect_start_x=(int)(pixel_per_tile_x*x);
			int rect_start_y=(int)(pixel_per_tile_y*y);
			int rect_end_x=(int)(pixel_per_tile_x*(x+1));
			int rect_end_y=(int)(pixel_per_tile_y*(y+1));
			Rect rect=make_rect_from_mincorner_width_height(pixel_per_tile_x*x,pixel_per_tile_y*y,1,pixel_per_tile_y);
			draw_rect(sub_screen,rect,0xffff0000);
			rect=make_rect_from_mincorner_width_height(pixel_per_tile_x*x,pixel_per_tile_y*y,pixel_per_tile_x,1);
			draw_rect(sub_screen,rect,0xffff0000);
		}
	}
	{
		Vec2 center=HaddamardProduct(pixel_per_tile,player->pos);
		Rect rect=make_rect_from_center_width_height(center,pixel_per_tile*0.1);
		draw_rect(sub_screen,rect,0x00ff0000);
	}
	{
		Vec2 pov_start_pos=HaddamardProduct(pixel_per_tile,player->pos+player->look_dir+Perp(player->look_dir)*player->fov);
		Rect rect=make_rect_from_center_width_height(pov_start_pos,pixel_per_tile*0.1);
		draw_rect(sub_screen,rect,0x00ffffff);
		Vec2 pov_end_pos=HaddamardProduct(pixel_per_tile,player->pos+player->look_dir-Perp(player->look_dir)*player->fov);
		rect=make_rect_from_center_width_height(pov_end_pos,pixel_per_tile*0.1);
		draw_rect(sub_screen,rect,0x00ffffff);
	}
}
Vec2 find_intersection_point(Map* map,Player* player,Vec2 dir)
{
//	float pixel_per_tile_x=(float)map_screen.width/(float)map->width;
//	float pixel_per_tile_y=(float)map_screen.height/(float)map->height;
//	Vec2 pixel_per_tile=vec2f(pixel_per_tile_x,pixel_per_tile_y);
	Vec2i current_tile=vec2i((int)player->pos.x,(int)player->pos.y);
	int dirv=1;
	if(dir.y<0)
	{
		dirv=-1;
	}
	int dirh=1;
	if(dir.x<0)
	{
		dirh=-1;
	}
	while(true)
	{
		int index=current_tile.x+current_tile.y*map->width;
		if(!map->arr[index])
		{
			//paint the point of intersection.
			Vec2 rel_pos=current_tile-player->pos;
			float t1=((rel_pos.y-(dirv-1)/2)/dir.y);
			float t=-1;
			float t2=((rel_pos.x-(dirh-1)/2)/dir.x);
			if(dir.x*t1-rel_pos.x>=0&&dir.x*t1- rel_pos.x<=1)
			{
				t=t1;
			}
			else
			{
				t=t2;
			}
			return t*dir+player->pos;
		}
		Vec2i right_tile=current_tile+vec2i(dirh,0);
		Vec2i down_tile=current_tile+vec2i(0,dirv);
		Vec2 right_tile_rel=right_tile-player->pos+vec2f(0.5,0.5);
		Vec2 down_tile_rel=down_tile-player->pos+vec2f(0.5,0.5);
		Vec2 perp=Perp(dir);
		if(es_abs(DotProduct(right_tile_rel,perp))<es_abs(DotProduct(down_tile_rel,perp)))
		{
			current_tile=right_tile;
		}
		else
		{
			current_tile=down_tile;
		}
	}
}
void go_game(Input* input, GameMemory* game_memory, read_file_type* read_file)
{
	Game_data* game_data=game_memory->game_data;
	screen_dim_=vec2f((float)game_memory->draw_context.screen->width,(float)game_memory->draw_context.screen->height);
	if(!game_data)
	{
		game_memory->game_data=push_struct(game_memory->const_buffer,Game_data);
		game_data=game_memory->game_data;
		Map* map=game_data->wall_tile_map=push_struct(game_memory->const_buffer,Map);
		map->width=100;
		map->height=100;
		map->arr=push_array(game_memory->const_buffer,bool,map->width*map->height);

		for(int i=1;i<map->height-1;i++)
		{
			for(int j=1;j<map->width-1;j++)
			{
				map->arr[j+i*map->width]=rand()/(float)RAND_MAX>0.2;
			}
		}
		map->arr[5*map->width+5]=1;
		game_data->player.pos=vec2f(5.5f,5.5f);
		game_data->player.look_dir=Normalize(vec2f(1,1));
		game_data->player.fov=2;
		render_queue=game_memory->render_queue=new_memory_buffer(game_memory->const_buffer,5*1024*1024);
		
	}
	clear_memory_buffer(render_queue);
	render_current_x=0;
	render_current_y=0;

	PictureBuffer* screen=game_memory->draw_context.screen;
	PictureBuffer map_screen=*screen;
	map_screen.width=screen->width-(screen->width/2+11);
	map_screen.picture+=(screen->width/2+11);
	Rect rect;
	rect.startx=(float)(screen->width/2-10);
	rect.endx=(float)(screen->width/2+11);
	rect.starty=0;
	rect.endy=(float)(screen->height);
	draw_rect(screen,rect,0xffffffff);
	render_current_x=screen->width/2+11;
	render_map(game_data->wall_tile_map,&game_data->player,&map_screen);

	Vec2 mouse_pos=input->mouse_pos;
	Rect map_screen_rect=make_rect_from_mincorner_width_height((float)(screen->width/2+11),0,(float)map_screen.width,(float)map_screen.height);
	if(is_vector_in_rect(map_screen_rect,mouse_pos))
	{
		Vec2 mouse_pos_in_map=vector_pos_rect_to_01(map_screen_rect,mouse_pos);
		Vec2 mouse_pos_in_tiles=HaddamardProduct(mouse_pos_in_map,vec2f((float)game_data->wall_tile_map->width,(float)game_data->wall_tile_map->height));
		int index=(int)mouse_pos_in_tiles.x+game_data->wall_tile_map->width*(int)mouse_pos_in_tiles.y;
		if(was_pressed(input->button_mouse_left))
		{
			game_data->wall_tile_map->arr[index]=!game_data->wall_tile_map->arr[index];
		}
	}

	Player* player=&game_data->player;
	Vec2 player_old_pos=player->pos;
	if(input->button_w.state)
	{
		game_data->player.pos+=player->look_dir*0.03;
	}
			
	if(input->button_s.state)
	{
		game_data->player.pos-=player->look_dir*0.03;
	}
	if(input->button_a.state)
	{
		Vec2 perp_look_dir=Perp(player->look_dir);
		player->look_dir+=perp_look_dir*0.04;
		player->look_dir=Normalize(player->look_dir);
	}
	if(input->button_d.state)
	{
			Vec2 perp_look_dir=Perp(player->look_dir);
		player->look_dir-=perp_look_dir*0.04;
		player->look_dir=Normalize(player->look_dir);
	}
	Map* map=game_data->wall_tile_map;
	int index=(int)player->pos.x+(int)player->pos.y*map->width;
	if(!map->arr[index])
	{
		player->pos=player_old_pos;
	}


	float pixel_per_tile_x=(float)map_screen.width/(float)map->width;
	float pixel_per_tile_y=(float)map_screen.height/(float)map->height;
	Vec2 pixel_per_tile=vec2f(pixel_per_tile_x,pixel_per_tile_y);
	Vec2 player_pos=player->pos;
	Vec2 intersection_point=find_intersection_point(map,player,player->look_dir);
	rect=make_rect_from_center_width_height(HaddamardProduct(intersection_point,pixel_per_tile),10,10);
	draw_rect(&map_screen,rect,0x22ff22ff);

	PictureBuffer wolf_screen=*screen;
	wolf_screen.width=screen->width-(screen->width/2+11);
	
	render_current_x=0;
	float fov_per_pixel=player->fov/wolf_screen.width;

	draw_rect(&wolf_screen,make_rect_from_mincorner_width_height(0,0,(float)wolf_screen.width,(float)wolf_screen.height/2.f),0x11222222);
	draw_rect(&wolf_screen,make_rect_from_mincorner_width_height(0,(float)wolf_screen.height/2.f,(float)wolf_screen.width,(float)wolf_screen.height/2.f),0x11335533);
	for(int x=0;x<wolf_screen.width;x++)
	{
		Vec2 dir=player->look_dir+(fov_per_pixel*(x-wolf_screen.width/2))*Perp(player->look_dir);
		Vec2 intersection_point=find_intersection_point(map,player,dir);
		Rect rect=make_rect_from_center_width_height(HaddamardProduct(intersection_point,pixel_per_tile),10,10);
		//draw_rect(&map_screen,rect,0x22ff22ff);
		
		float z=DotProduct(intersection_point-player->pos,player->look_dir);
		//paint the screen so 0.1 dist will be full screen;
		float height=1.f/z*wolf_screen.height;
		if(height>wolf_screen.height)
		{
			height=(float)wolf_screen.height;
		}
		Rect wolf_rect=make_rect_from_center_width_height((float)x,(float)wolf_screen.height/2.f,1.f,height);
		if((int)(intersection_point.x*10+0.5)%2)
		{
			draw_rect(&wolf_screen,wolf_rect,0x22ff22ff);
		}
		else if((int)(intersection_point.y*10+0.5)%2)
		{
			draw_rect(&wolf_screen,wolf_rect,0xffff2222);
		}
		else
		{
			draw_rect(&wolf_screen,wolf_rect,0x22ffffff);
		}
	}
}

