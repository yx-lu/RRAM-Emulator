# RRAM-Emulator

本项目采用MIT license.

## rram.h

rram.h实现了一个rram的基本功能, 假设大小为$512*512$

### rram块的抽象

* data是一个512*512bit的存储空间, 任何两行两列都可以做x&=!y
* x是一个input buffer, 写入data的数据需要经过x
* y是一个output buffer, 从data读出的数据需要经过y
* lp是一个512bit的数组, 表示在做行并行的时候并行执行哪些列上的内容, 默认值为全1
* cp是一个512bit的数组, 表示在做列并行的时候并行执行哪些行上的内容, 默认值为全1

### 支持运算的解释

(BIT被定义为类型bitset<512>)

(LP集合指lp值为1的下标的集合, CP集合同理)

* 初始化
    * RRAM(): 缺省初始值的申请一个RRAM块, 其初始值会被赋为全1
    * RRAM(BIT init[Size]): 申请一个RRAM块, 其初始值会被赋为init中的内容
    * ~RRAM(): 释放一个RRAM块
* 基本赋值
    * void changelp(BIT b): 更改lp为b
    * void changecp(BIT b): 更改cp为b
    * void lineset(int i): 将行i的lp为1的那些列置为1
    * void linereset(int i): 将行i的lp为1的那些列置为0
    * void columnset(int i): 将列i的cp为1的那些列置为1
    * void columnreset(int i): 将列i的cp为1的那些列置为0
* 基本运算
    * void lineop(int i,int j): $\forall k\in LP,data[i][k]\&=!data[j][k]$
    * void lineop(int i,int j1,int j2): $\forall k\in LP,data[i][k]\&=(!data[j1][k])\&(!data[j2][k])$
    * void columnop(int i,int j): $\forall k\in CP,data[k][i]\&=!data[k][j]$
    * void columnop(int i,int j1,int j2): $\forall k\in CP,data[k][i]\&=(!data[k][j1])\&(!data[k][j2])$
    * void mult(): $\forall i,y[i]=x\&data[i].any();$
* 缓冲区操作
    * void line2buf(int i): 把行i复制给y
    * void column2buf(int i): 把列i复制给y
    * void buf2line(int i): 把x复制给行i
    * void buf2column(int i): 把x复制给列i
    * void buf2buf(): 把x复制给y
* 简单读写
    * void write_buf(BIT b,bool io=true): 把b写入x
    * BIT read_buf(bool io=true): 读出y
    * friend void trans_buf(RRAM &A,RRAM &B): 将A.y复制给B.x
* 封装过的操作
    * friend void transll(RRAM &A,int i,RRAM &B,int j): 把A的行i复制到B的行j
    * friend void translc(RRAM &A,int i,RRAM &B,int j): 把A的行i复制到B的列j
    * friend void transcl(RRAM &A,int i,RRAM &B,int j): 把A的列i复制到B的行j
    * friend void transcc(RRAM &A,int i,RRAM &B,int j): 把A的列i复制到B的列j
    * BIT readline(int i): 读出行i
    * BIT readcolumn(int i): 读出列i
    * void writeline(BIT b,int i): 把b写入行i
    * void writecolumn(BIT b,int i): 把b写入列i
* void Printinfo(FILE* file=stderr): 在程序结束时运行这个函数, 将输出rram的使用情况

### 不符合实际的假设

* 假定将某行(列)复制到y以及把x复制到某行(列)都需要两次操作(实际取决于优化)
* 假定在任何buffer之间均可传输数据(实际只有物理排布相邻的可以)

* 假定在不同的rram块的buffer之间传输信息都可以做到, 而且不需要经过cpu, 也就是不计io次数

## rram_allocator.h

一个简单的rram调度器, 使你可以不再直接面对rram进行操作.

其核心是一个称为row​的类型, 程序员可以将他看做一个大小为512bit的向量. 而其本质上是某一个rram的两个行, 分别存储这个向量和这个向量取反.

### 支持运算的解释

* row(int v=0): row的构造函数, 新申请一个row, 初始化为全0(若v=1则为全1)
* row(const row &y): row的复制构造函数, 采用**浅复制**
* friend row operator !(const row &x): 将一个row取反, 返回一个row, 采用**浅复制**
* friend row deepcopy(const row &y): **深复制**一个row并返回, 也就是新申请了空间
* void set(): 将一个row置为1
* void reset(): 将一个row置为0
* void write(BIT b): 将b写入row
* BIT read(): 读出row中的内容
* void operator |=(const row &y): x|=y
* void operator &=(const row &y): x&=y
* void operator ^=(const row &y): x^=y

###实现细节

```
x^=y:
    z=1
    z&=!y
    z&=x
    w=1
    w&=!x
    w&=y
    nz=1
    nz&=!z
    nw=1
    nw&=!w
    x=1
    x&=!nz
    x&=!nw
    nx=1
    nx&=x
----------------
x&=y
    x&=!ny
    nx=1
    nx&=!x
----------------
x|=y
    nx&=!y
    x=1
    x&=!nx
```

## test

### sha3.cpp

### sha2.cpp

### sha1.cpp

### md5.cpp