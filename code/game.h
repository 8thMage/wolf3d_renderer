#include "types.h"
#include "macros.h"
#include "vector.cpp"
#include "memorybuffer.cpp"
#include "winmain.h"
struct Draw_context
{
	PictureBuffer* screen;
	Matrix4x3 transform;
	s32 top_z;
};
struct Render_queue
{
	MemoryBuffer* queue;
};
struct Ball;
struct MBalls
{
	Vec2* pos;
	float* rad;
	int len;
};
struct Game_data
{
	MBalls mbs;
	Ball* balls;
	int balls_len;
};

struct GameMemory
{
	MemoryBuffer* const_buffer;
	MemoryBuffer* temp_buffer;
	Draw_context draw_context;
	MemoryBuffer* render_queue;
	Game_data* game_data;
	bool paused;
};
typedef void goGametype (Input*,GameMemory*,read_file_type* read_file);
