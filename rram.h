#include<cstdio>
#include<bitset>
#include<cassert>
const int RRAM_SIZE=512;
int INPUT_CPU_FROM_RRAM,OUTPUT_CPU_TO_RRAM;
int RRAM_PARALLEL_POSITION_SET,RRAM_OPERATION_MAGIC,RRAM_OPERATION_MATRIX_VECTOR_MULT,RRAM_CROSS_REGION_TRANSFER;
int RRAM_USED,RRAM_MAXUSED;
typedef std::bitset<RRAM_SIZE> BIT;
#define Input INPUT_CPU_FROM_RRAM
#define Output OUTPUT_CPU_TO_RRAM
#define Posset RRAM_PARALLEL_POSITION_SET
#define Magic RRAM_OPERATION_MAGIC
#define Mult RRAM_OPERATION_MATRIX_VECTOR_MULT
#define Trans RRAM_CROSS_REGION_TRANSFER
#define Use RRAM_USED
#define Maxuse RRAM_MAXUSED
#define Size RRAM_SIZE
struct RRAM
{
	BIT data[Size],x,y,lp,cp;
	//x:input buffer
	//y:output buffer
	//data:data
	//lp:line parallel positions
	//cp:column parallel positions
	
	
	//initialization
	RRAM()//init=all 1
	{
		Use++;if (Maxuse<Use) Maxuse=Use;x.set();y.set();
		for (int i=0;i<Size;i++) data[i].set();
		lp.set();cp.set();
	}
	RRAM(BIT init[Size])
	{
		Use++;if (Maxuse<Use) Maxuse=Use;x.set();y.set();
		for (int i=0;i<Size;i++) data[i]=init[i];
		lp.set();cp.set();
	}
	~RRAM()
	{
		Use--;
	}
	
	
	//set operation
	void changelp(BIT b){Posset++;lp=b;}
	void changecp(BIT b){Posset++;cp=b;}
	void lineset(int i){for (int k=0;k<Size;k++) if (lp[k]) data[i][k]=1;}
	void linereset(int i){for (int k=0;k<Size;k++) if (lp[k]) data[i][k]=0;}
	void columnset(int i){for (int k=0;k<Size;k++) if (lp[k]) data[k][i]=1;}
	void columnreset(int i){for (int k=0;k<Size;k++) if (lp[k]) data[k][i]=0;}
	
	
	//basic calculation
	void lineop(int i,int j)//li&=!lj
	{
		if ((i<0)||(i>=Size)||(j<0)||(j>=Size)||(i==j)) {fprintf(stderr,"Index overflow!");assert(0);}
		Magic++;
		for (int k=0;k<Size;k++) if (lp[k]) data[i][k]=data[i][k]&(!data[j][k]);
	}
	void lineop(int i,int j1,int j2)//li&=(!lj1)&(!lj2)
	{
		if ((i<0)||(i>=Size)||(j1<0)||(j1>=Size)||(j2<0)||(j2>Size)||(i==j1)||(i==j2)) {fprintf(stderr,"Index overflow!");assert(0);}
		Magic++;
		for (int k=0;k<Size;k++) if (lp[k]) data[i][k]=data[i][k]&(!data[j1][k])&(!data[j2][k]);
	}
	void columnop(int i,int j)//ci&=!cj
	{
		if ((i<0)||(i>=Size)||(j<0)||(j>=Size)||(i==j)) {fprintf(stderr,"Index overflow!");assert(0);}
		Magic++;
		for (int k=0;k<Size;k++) if (cp[k]) data[k][i]=data[k][i]&(!data[k][j]);
	}
	void columnop(int i,int j1,int j2)//ci&=(!cj1)&(!cj2)
	{
		if ((i<0)||(i>=Size)||(j1<0)||(j1>=Size)||(j2<0)||(j2>Size)||(i==j1)||(i==j2)) {fprintf(stderr,"Index overflow!");assert(0);}
		Magic++;
		for (int k=0;k<Size;k++) if (cp[k]) data[k][i]=data[k][i]&(!data[k][j1])&(!data[k][j2]);
	}
	void mult()//y=data*x (and mult)
	{
		Mult++;
		y.reset();
		for (int i=0;i<Size;i++) y[i]=(x&data[i]).any();
	}
	
	
	//basic buffer operation
	void line2buf(int i)//output line_i to y
	{
		Magic+=2;
		y=data[i];
	}
	void column2buf(int i)//output column_i to y
	{
		Magic+=2;
		for (int k=0;k<Size;k++) y[k]=data[k][i];
	}
	void buf2line(int i)//input line_i from x
	{
		Magic+=2;
		data[i]=x;
	}
	void buf2column(int i)//input column_i from x
	{
		Magic+=2;
		for (int k=0;k<Size;k++) data[k][i]=x[k];
	}
	void buf2buf()//y=x
	{
		Magic+=2;
		y=x;
	}
	
	
	//basic io
	void write_buf(BIT b,bool io=true)//input x from cpu
	{
		Output++;
		x=b;
	}
	BIT read_buf(bool io=true)//output y to cpu
	{
		Input++;
		return y;
	}
	friend void trans_buf(RRAM &A,RRAM &B)//B.x=A.y
	{
		Trans++;
		B.x=A.y;
	}
	
	
	//packaged buffer op
	friend void transll(RRAM &A,int i,RRAM &B,int j)
	{
		A.line2buf(i);
		trans_buf(A,B);
		B.buf2line(j);
	}
	friend void translc(RRAM &A,int i,RRAM &B,int j)
	{
		A.line2buf(i);
		trans_buf(A,B);
		B.buf2column(j);
	}
	friend void transcl(RRAM &A,int i,RRAM &B,int j)
	{
		A.column2buf(i);
		trans_buf(A,B);
		B.buf2line(j);
	}
	friend void transcc(RRAM &A,int i,RRAM &B,int j)
	{
		A.column2buf(i);
		trans_buf(A,B);
		B.buf2column(j);
	}
	
	
	//packaged io
	BIT readline(int i)
	{
		line2buf(i);
		return read_buf();
	}
	BIT readcolumn(int i)
	{
		column2buf(i);
		return read_buf();
	}
	void writeline(BIT b,int i)
	{
		write_buf(b);
		buf2line(i);
	}
	void writecolumn(BIT b,int i)
	{
		write_buf(b);
		buf2column(i);
	}
};

void Printinfo(FILE* file=stderr)//run this before ending
{
	fprintf(file,"RRAM_USED: %d\n",RRAM_USED);
	fprintf(file,"RRAM_MAXUSED: %d\n",RRAM_MAXUSED);
	fprintf(file,"RRAM_PARALLEL_POSITION_SET: %d\n",RRAM_PARALLEL_POSITION_SET);
	fprintf(file,"INPUT_CPU_FROM_RRAM: %d\n",INPUT_CPU_FROM_RRAM);
	fprintf(file,"OUTPUT_CPU_TO_RRAM: %d\n",OUTPUT_CPU_TO_RRAM);
	fprintf(file,"RRAM_OPERATION_MAGIC: %d\n",RRAM_OPERATION_MAGIC);
	fprintf(file,"RRAM_OPERATION_MATRIX_VECTOR_MULT: %d\n",RRAM_OPERATION_MATRIX_VECTOR_MULT);
	fprintf(file,"RRAM_CROSS_REGION_TRANSFER: %d\n",RRAM_CROSS_REGION_TRANSFER);
}

