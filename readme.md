# readme

## rram.h

rram.h实现了一个rram的基本功能, 假设大小为$512*512$

其主要用于计算行列之间的x\&=!y​以及计算矩阵向量and乘

一个rram块的构造被抽象为:

* data是一个512*512bit的存储空间, 任何两行两列都可以做x&=!y
* x是一个input buffer, 写入data的数据需要经过x
* y是一个output buffer, 从data读出的数据需要经过y

对于一个矩阵向量乘, 从x中读入向量将结果输出到y

假定将某行(列)复制到y以及把x复制到某行(列)都只需要一次操作(实际上需要至少2次, 取决于优化)

假定在不同的rram块的buffer之间传输信息都可以做到, 而且不需要经过cpu, 也就是不计io次数

Printinfo函数将输出使用rram的情况

## dispatch.h

TODO

dispatch.h预计将实现一个简单包装完成下列的运算:

```c++
struct row//从外部看是存储一个512bit的向量(本质上是某一个rram的两个行存)

row x,y
x.set()//1
x.reset()//1
x=!x//0
x&=y//3
x|=y//3
x^=y//?
x&=!y//3
x|=!y//3
x^=!y//?
perm(A,x)
```

其内部的调度是对于每个向量x在某个rram块中存储两个行x和!x, 这样每个操作周期数如代码中所示.

一个trick是!x返回一个row, 两个指针翻转

还有一个trick是定义一个rram块的时候先置1, 这样算not能1个周期(其实是不管算什么都少一个周期)

```
x^=y

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
15周期
----------------
x&=y

x&=!ny
nx=1
nx&=!x
3周期
----------------
x|=y

nx&=!y
x=1
x&=!nx
3周期
```

传统方法一个nor是3周期, xor需要6个nor

## test

### sha3.cpp

### sha2.cpp

### sha1.cpp

### md5.cpp