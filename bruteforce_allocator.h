#include"rram.h"
#include<vector>
const int MAX_USED=RRAM_SIZE-8;
std::vector<RRAM*> pool;
int lineused=MAX_USED;
struct row
{
	BIT *fore,*back;
	
	
	//init and copy
	row(int v=0)
	{
		fore=new BIT;
		back=new BIT;
		if (v==1) {fore->set();back->reset();}
		else if (v==0) {fore->reset();back->set();}
		else if (v==-1) {fore->set();back->set();}
		else assert(0);
	}
	row(const row &y)
	{
		fore=y.fore;
		back=y.back;
	}
	friend row operator !(const row &x)
	{
		row y(x);
		std::swap(y.fore,y.back);
		return y;
	}
	friend row deepcopy(const row &y)
	{
		row x(0);
		x|=y;
		return x;
	}
	
	
	//set and reset
	void set()
	{
		fore->set();
		back->reset();
	}
	void reset()
	{
		fore->reset();
		back->set();
	}
	
	//read and write
	void write(BIT b)
	{
		*fore=b;
		*back=(~b);
	}
	BIT read(){return *fore;}
	
	
	//calculation
	void operator |=(const row &y)
	{
		*fore|=*(y.fore);
		*back=~(*fore);
	}
	void operator &=(const row &y)
	{
		*fore&=*(y.fore);
		*back=~(*fore);
	}
	void operator ^=(const row &y)
	{
		*fore^=*(y.fore);
		*back=~(*fore);
	}
};

