#include "game.h"
#include "render.h"
void go_game(Input* input, GameMemory* game_memory, read_file_type* read_file)
{
	if(!game_memory->game_data)
	{
		game_memory->game_data=push_struct(game_memory->const_buffer,Game_data);
		Game_data* game_data=game_memory->game_data;
		game_data->mbs_len=1;
		game_data->mbs=push_array(game_memory->const_buffer,MBall,game_data->mbs_len);
		game_memory->render_queue=new_memory_buffer(game_memory->const_buffer,10 MB);
	}

}
