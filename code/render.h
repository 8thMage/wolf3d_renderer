enum Render_tag
{
	RT_MBalls,
	RT_Balls,
	RT_Triangle
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
	int len;
};
struct RC_Triangle
{
	Render_tag tag;
	Vec2 vrts[3];
};
