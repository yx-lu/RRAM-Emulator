#include <iostream>
#include <algorithm>
#include "rram_allocator.h"

BIT shifter_bit[Size];
RRAM *shifter[Size];

BIT rotater_bit[Size];
RRAM *rotater[Size];

BIT left_shifter_bit[Size];
RRAM *left_shifter[Size];

BIT allset_bit[Size];
RRAM *allset;
void Initializer() {
	for (int i = 0; i < Size; i++) shifter_bit[i].set(i);
	shifter[0] = new RRAM(shifter_bit);
	for (int d = 0; d < Size - 1; d++) {
		for (int i = 0; i < Size; i++) shifter_bit[i]<<=1;
		shifter[d+1] = new RRAM(shifter_bit);
	}
	
	for (int i = 0; i < Size; i++) rotater_bit[i].set(i);
	rotater[0] = new RRAM(rotater_bit);
	for (int d = 0; d < Size - 1; d++) {
		for (int i = 0; i < Size; i++) rotater_bit[i]<<=1;
		rotater_bit[d].set(0);
		rotater[d+1] = new RRAM(rotater_bit);
	}
	for (int i = 0; i < Size; i++) left_shifter_bit[i].set(i);
	left_shifter[0] = new RRAM(left_shifter_bit);
	for (int d = 0; d < Size - 1; d++) {
		for (int i = 0; i < Size; i++) left_shifter_bit[i]>>=1;
		left_shifter[d+1] = new RRAM(left_shifter_bit);
	}
	for (int i = 0; i < Size; i++) allset_bit[i].set(0);
	allset= new RRAM(allset_bit);
}

void shift(int d, row &r1, row &r2) {
	r1.fr->line2buf(r1.fore);
	trans_buf(*r1.fr, *shifter[d]);
	shifter[d]->mult();
	trans_buf(*shifter[d], *r2.fr);
	r2.fr->buf2line(r2.fore);
	r2.fr->lineset(r2.back);
	r2.fr->lineop(r2.back,r2.fore);
}

void rotate(int d, row &r1, row &r2) {
	r1.fr->line2buf(r1.fore);
	trans_buf(*r1.fr, *rotater[d]);
	rotater[d]->mult();
	trans_buf(*rotater[d], *r2.fr);
	r2.fr->buf2line(r2.fore);
	r2.fr->lineset(r2.back);
	r2.fr->lineop(r2.back,r2.fore);
}


void leftshift(int d,row &r1, row &r2) {
	r1.fr->line2buf(r1.fore);
	trans_buf(*r1.fr, *left_shifter[d]);
	left_shifter[d]->mult();
	trans_buf(*left_shifter[d], *r2.fr);
	r2.fr->buf2line(r2.fore);
	r2.fr->lineset(r2.back);
	r2.fr->lineop(r2.back,r2.fore);
}


void allsetter(row &r1, row &r2){	
	r1.fr->line2buf(r1.fore);
	trans_buf(*r1.fr, *allset);
	allset->mult();
	trans_buf(*allset, *r2.fr);
	r2.fr->buf2line(r2.fore);
	r2.fr->lineset(r2.back);
	r2.fr->lineop(r2.back,r2.fore);
}

void add(row &r1, row &r2){
	row r3;
	row r4;
	row r5;
	row r6;
	row r7;
	r3.set();
	r4.set();
	r3&=r1;
	r4&=r2;		
	for(int i=0;i<Size;++i){
	r5.set();
	r6.set();
	r5&=r3;
	r5^=r4;
	r6&=r3;
	r6&=r4;	
	leftshift(1,r6,r4);
	r3.set();
	r3&=r5;
	}
	r1.set();
	r1&=r5;
}

void substract(row &r1, row &r2){
	row r3;
	r3.set();
	r3&=!r2;
	add(r1,r3);
	r3.write(BIT(1));
	add(r1,r3);
}

void mutiply(row &r1, row &r2){
	row r3;
	row r4;
	r3.set();
	r4.set();
	r3&=r1;
	r4&=r2;	
	row r5;
	r5.reset();
	row rarray[Size];
	for(int i=0;i<Size;++i){
	rarray[i].write(BIT(1<<i));
	}	
	for(int i=0;i<Size;++i){
	row r6;
	r6.write(BIT(1));
	r6&=r4;
	allsetter(r6,r6);
	row r7;
	r7.set();
	r7&=r3;
	r7&=r6;	
	add(r5,r7);
	leftshift(1,r3,r3);
	shift(1,r4,r4);	
	}
	r1.set();
	r1&=r5;	
}

