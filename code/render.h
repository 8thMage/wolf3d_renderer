enum Render_tag
{
	RT_MBalls,
};

struct RC_MBalls
{
	Render_tag tag;
	Vec2* mballs_position;
	float* mballs_radius;
	int len;
};
