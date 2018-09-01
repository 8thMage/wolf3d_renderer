enum Render_tag
{
	RT_cube_array,
	RT_cube_dim_array,
	RT_transform,
};

struct RC_render_cube_array
{
	Render_tag tag;
	int length;
	Vec3i* poses;
	umo* VB;
};
struct RC_render_cube_dim_array
{
	Render_tag tag;
	int length;
	Vec3i* poses_dims_f;
	u8* poses_dims;
	Vec3i position;
	Vec3 start_bounding_box;
	Vec3 end_bounding_box;
	umo* VB;
};
struct RC_transform
{
	Render_tag tag;
	Matrix4x3 transform;
};

