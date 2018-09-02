enum Render_tag
{
	RT_MBalls,
	RT_Triangle
};

struct RC_MBalls
{
	Render_tag tag;
	Vec2* mballs_position;
	float* mballs_radius;
	int len;
};
struct RC_Triangle
{
	Render_tag tag;
	Vec2 vrts[3];
};
