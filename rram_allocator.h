#include"rram.h"
#include<vector>
#include<iostream>
const int MAX_USED=RRAM_SIZE-8;
std::vector<RRAM*> pool;
int lineused=MAX_USED;
struct row
{
	RRAM *fr;
	int fore,back;
	
	
	//init and copy
	row(int v=0)
	{
		if (lineused==MAX_USED) {RRAM* nw=new RRAM;pool.push_back(nw);lineused=0;}
		fr=pool.back();
		fore=lineused++;
		back=lineused++;
		if (v==1) fr->linereset(back);
		else if (v==0) fr->linereset(fore);
		else if (v==-1) {}
		else assert(0);
	}
	row(const row &y)
	{
		fr=y.fr;
		fore=y.fore;
		back=y.back;
	}
	static void new_chunk() {RRAM* nw=new RRAM;pool.push_back(nw);lineused=0;}
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
		fr->lineset(fore);
		fr->linereset(back);
	}
	void reset()
	{
		fr->linereset(fore);
		fr->lineset(back);
	}
	
	//read and write
	void write(BIT b)
	{
		fr->writeline(b,fore);
		fr->lineset(back);
		fr->lineop(back,fore);
	}
	BIT read(){return fr->readline(fore);}
	
	
	//calculation
	void operator |=(const row &y)
	{
		if (y.fr==fr) fr->lineop(back,y.fore);
		else
		{
			transll(*y.fr,y.fore,*fr,RRAM_SIZE-1);
			fr->lineop(back,RRAM_SIZE-1);
		}
		fr->lineset(fore);
		fr->lineop(fore,back);
		//nx&=!y
		//x=1
		//x&=!nx
	}
	void operator &=(const row &y)
	{
		if (y.fr==fr) fr->lineop(fore,y.back);
		else
		{
			transll(*y.fr,y.back,*fr,RRAM_SIZE-1);
			fr->lineop(fore,RRAM_SIZE-1);
		}
		fr->lineset(back);
		fr->lineop(back,fore);
		//x&=!ny
		//nx=1
		//nx&=!x
	}
	void operator ^=(const row &y)
	{
		int yfore,yback;
		if (y.fr==fr) {yfore=y.fore;yback=y.back;}
		else
		{
			transll(*y.fr,y.fore,*fr,RRAM_SIZE-1);
			yfore=RRAM_SIZE-1;yback=RRAM_SIZE-2;
			fr->lineset(yback);
			fr->lineop(yback,yfore);
		}
		int z=RRAM_SIZE-3,nz=RRAM_SIZE-4,w=RRAM_SIZE-5,nw=RRAM_SIZE-6;
		fr->lineset(z);
		fr->lineop(z,yfore,back);
		fr->lineset(w);
		fr->lineop(w,fore,yback);
		fr->lineset(back);
		fr->lineop(back,z,w);
		fr->lineset(fore);
		fr->lineop(fore,back);
		//z=1
		//z&=!y
		//z&=x
		//w=1
		//w&=!x
		//w&=y
		//nz=1
		//nz&=!z
		//nw=1
		//nw&=!w
		//x=1
		//x&=!nz
		//x&=!nw
		//nx=1
		//nx&=x
	}
};
