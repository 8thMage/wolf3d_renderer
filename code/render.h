enum Render_tag
{
	RT_MBalls,
	RT_Balls,
	RT_Dust,
	RT_Triangle,
	RT_Triangle_normals,
	RT_start_tri_arr,
	RT_rect,
	RT_end_rect
};

struct RC_MBalls
{
	Render_tag tag;
	Vec2* mballs_position;
	float* mballs_radius;
	int len;
};
struct RC_Balls
{
	Render_tag tag;
	Vec2* pos;
	float radius;
	int len;
};
struct RC_Dust
{
	Render_tag tag;
	Vec2* pos;
	int len;
};
struct RC_Triangle
{
	Render_tag tag;
	Vec3 vrts[3];
};
struct Render_triangle
{
	Vec3 vrt0;
	Vec3 normal0;
	Vec3 vrt1;
	Vec3 normal1;
	Vec3 vrt2;
	Vec3 normal2;
};
struct RC_Triangle_normals
{
	Render_tag tag;
	Vec3 vrts[3];
	Vec3 normals[3];
};

struct RC_rect
{
	Render_tag tag;
	Rect rect;
	u32 color;
};
struct RC_end_rect
{
	Render_tag tag;
};
struct RC_start_tri_arr
{
	Render_tag tag;
	int count;
	unsigned int* vbo;
};
