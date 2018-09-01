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
struct MC_chunk
{
	Vec3i position;
	int current_write_pos=0;
	Vec3i* poses;
	umo VB;
	Vec3 start_bounding_box;
	Vec3 end_bounding_box;
};
struct MC_chunk_optimised
{
	Vec3i position;
	int current_write_pos=0;
	Vec3i* poses_dims_f;
	u8* poses_dims;
	Vec3 start_bounding_box;
	Vec3 end_bounding_box;
	umo VB;
};

struct GameMemory
{
	MemoryBuffer* const_buffer;
	MemoryBuffer* temp_buffer;
	Draw_context draw_context;
	MemoryBuffer* render_queue;
	bool paused;
//	MC_chunk chunk[32*32];
	MC_chunk_optimised opt_chunk[32*32];
	MC_chunk_optimised opt_chunk_combined;
};
typedef void goGametype (Input*,GameMemory*,read_file_type* read_file);
