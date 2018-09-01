enum Render_tag
{
	RT_MBalls,
};

struct RC_MBalls
{
	Render_tag* tag;
	MBall* mballs;
	int len;
};
