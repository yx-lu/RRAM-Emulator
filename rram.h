#include<cstdio>
#include<bitset>
#include<cassert>
const int RRAM_SIZE=512;
int INPUT_CPU_FROM_RRAM,OUTPUT_CPU_TO_RRAM,RRAM_OPERATION_MAGIC,RRAM_OPERATION_MATRIX_VECTOR_MULT,RRAM_CROSS_REGION_TRANSFER;
int RRAM_USED,RRAM_MAXUSED;
typedef std::bitset<RRAM_SIZE> BIT;
#define Input INPUT_CPU_FROM_RRAM
#define Output OUTPUT_CPU_TO_RRAM
#define Magic RRAM_OPERATION_MAGIC
#define Mult RRAM_OPERATION_MATRIX_VECTOR_MULT
#define Trans RRAM_CROSS_REGION_TRANSFER
#define Use RRAM_USED
#define Maxuse RRAM_MAXUSED
#define Size RRAM_SIZE
struct RRAM
{
	BIT data[Size],x,y;
	//x:input buffer
	//y:output buffer
	//data:data
	

	//initialization
	RRAM()
	{
		Use++;if (Maxuse<Use) Maxuse=Use;x.reset();y.reset();
		for (int i=0;i<Size;i++) data[i].reset();
	}
	RRAM(BIT init[Size])
	{
		Use++;if (Maxuse<Use) Maxuse=Use;x.reset();y.reset();
		for (int i=0;i<Size;i++) data[i]=init[i];
	}
	~RRAM()
	{
		Use--;
	}
	
	
	//basic calculation
	void lineop(int i,int j)//linei &= ! linej
	{
		if ((i<0)||(i>=Size)||(j<0)||(j>=Size)||(i==j)) {fprintf(stderr,"Index overflow!");assert(0);}
		Magic++;
		for (int k=0;k<Size;k++) data[i][k]=data[i][k]&(!data[j][k]);
	}
	void columnop(int i,int j)//columni &= ! columnj
	{
		if ((i<0)||(i>=Size)||(j<0)||(j>=Size)||(i==j)) {fprintf(stderr,"Index overflow!");assert(0);}
		Magic++;
		for (int k=0;k<Size;k++) data[k][i]=data[k][i]&(!data[k][j]);
	}
	void mult()//calculate this.y=this.x * this.data (and mult)
	{
		Mult++;
		y.reset();
		for (int i=0;i<Size;i++) y|=x&data[i];
	}
	
	
	//basic buffer operation
	void line2buf(int i)//output line_i to y
	{
		Magic++;//indead it needs more than 1 Magicop
		y=data[i];
	}
	void column2buf(int i)//output column_i to y
	{
		Magic++;
		for (int k=0;k<Size;k++) y[k]=data[k][i];
	}
	void buf2line(int i)//input line_i from x
	{
		Magic++;
		data[i]=x;
	}
	void buf2column(int i)//input column_i from x
	{
		Magic++;
		for (int k=0;k<Size;k++) data[k][i]=x[k];
	}
	void buf2buf()//y=x
	{
		Magic++;
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
	fprintf(file,"INPUT_CPU_FROM_RRAM: %d\n",INPUT_CPU_FROM_RRAM);
	fprintf(file,"OUTPUT_CPU_TO_RRAM: %d\n",OUTPUT_CPU_TO_RRAM);
	fprintf(file,"RRAM_OPERATION_MAGIC: %d\n",RRAM_OPERATION_MAGIC);
	fprintf(file,"RRAM_OPERATION_MATRIX_VECTOR_MULT: %d\n",RRAM_OPERATION_MATRIX_VECTOR_MULT);
	fprintf(file,"RRAM_CROSS_REGION_TRANSFER: %d\n",RRAM_CROSS_REGION_TRANSFER);
}

