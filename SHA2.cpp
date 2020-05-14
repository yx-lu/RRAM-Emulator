#include <iostream>
#include <algorithm>
#include "rram_allocator.h"

const int BLOCK_SIZE = 512;
const int OUTPUT_LENGTH = 256;
const int WIDTH = 1600;
const int MAXLEN = BLOCK_SIZE*64;
const int roffset[5][5] = 
{ { 0,36, 3,41,18},
  { 1,44,10,45, 2},
  {62, 6,43,15,61},
  {28,55,25,21,56},
  {27,20,39, 8,14}
};

const unsigned int k[64]={
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};


BIT shifter_bit[Size];
RRAM *shifter[Size];

BIT rotater_bit[Size];
RRAM *rotater[Size];

BIT left_shifter_bit[Size];
RRAM *left_shifter;
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
	for (int i = 0; i < Size-1; i++) left_shifter_bit[i].set(i+1);
	left_shifter=new RRAM(left_shifter_bit);
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


void leftshift(row &r1, row &r2) {
	r1.fr->line2buf(r1.fore);
	trans_buf(*r1.fr, *left_shifter);
	left_shifter->mult();
	trans_buf(*left_shifter, *r2.fr);
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
	for(int i=0;i<32;++i){
	r5.set();
	r6.set();
	r5&=r3;
	r5^=r4;
	r6&=r3;
	r6&=r4;	
	leftshift(r6,r4);
	r3.set();
	r3&=r5;
	}
	r1.set();
	r1&=r5;
}

int len = 0;
char plain[MAXLEN];
std::string bitplain;
void Plaintext_Input() {
	while (len<MAXLEN && scanf("%c",&plain[len])!=EOF) len++;
	if (len==MAXLEN) {
		std::cerr << "Error: Input too long. Maximum Length: "
				  << MAXLEN << std::endl;
		exit(0);
	}	
}
void Padding() {
	bitplain = "";
	for (int i = 0; i < len; i++) {
		bitplain += std::bitset<8>((unsigned)plain[i]).to_string();
	}
	bitplain += "1";
	int delta = ((-(int)bitplain.length()-64)%BLOCK_SIZE+BLOCK_SIZE)%BLOCK_SIZE;
	bitplain += std::string(delta,'0');
	bitplain += std::bitset<64>(len).to_string();
}

row s0;
row s1;
row s2;
row temp1;
row temp2;
row ch;
row maj;
void Round(row* horigin, row* w) {
	row h[8];
	for(int i=0;i<8;++i){	
	h[i].set();
	h[i]&=horigin[i];
	}
	for (int i = 16; i < 64; i++) {
	rotate(7,w[i-15],s0);
	rotate(18,w[i-15],s1);
	s0^=s1;
	shift(3,w[i-15],s1);
	s0^=s1;
	rotate(17,w[i-2],s1);
	rotate(19,w[i-2],s2);
	s1^=s2;
	shift(10,w[i-15],s2);
	s1^=s2;
	w[i].reset();
	w[i]|=w[i-16];
	add(w[i],s0);
	add(w[i],w[i-7]);
	add(w[i],s1);
	}
	for (int i=0 ;i< 64;++i){
	rotate(6,h[4],s1);
	rotate(11,h[4],s2);
	s1^=s2;
	rotate(25,h[4],s2);
	s1^=s2;
	ch.set();
	ch&=h[4];
	ch&=h[5];
	s2.set();
	s2&=!h[4];
	s2&=h[6];
	ch^=s2;
	temp1.set();
	temp1&=h[7];
	add(temp1,s1);
	add(temp1,ch);
	s2.write(BIT(k[i]));
	add(temp1,s2);	
	add(temp1,w[i]);
	rotate(2,h[0],s0);
	rotate(13,h[0],s2);
	s0^=s2;
	rotate(22,h[0],s2);
	s0^=s2;
	s2.set();
	s2&=h[0];
	s2&=h[2];
	maj.set();
	maj&=h[0];
	maj&=h[1];
	maj^=s2;
	s2.set();
	s2&=h[1];
	s2&=h[2];
	maj^=s2;
	temp2.set();
	temp2&=s0;
	add(temp2,maj);
        h[7].set();
	h[7]&=h[6];
        h[6].set();
	h[6]&=h[5];
        h[5].set();
	h[5]&=h[4];
        h[4].set();
	h[4]&=h[3];
	add(h[4],temp1);
        h[3].set();
	h[3]&=h[2];
        h[2].set();
	h[2]&=h[1];
        h[0].set();
	h[0]&=temp1;
	add(h[0],temp2);
	}
	for(int i=0;i<8;++i){	
	add(horigin[i],h[i]);
	}
}

void SHA2() {
	Padding();
	row h[8];
	h[0].write(BIT(0x6a09e667));
	h[1].write(BIT(0xbb67ae85));
	h[2].write(BIT(0x3c6ef372));
	h[3].write(BIT(0xa54ff53a));
	h[4].write(BIT(0x510e527f));
	h[5].write(BIT(0x9b05688c));
	h[6].write(BIT(0x1f83d9ab));
	h[7].write(BIT(0x5be0cd19));
	row tmp[64];
	int n = bitplain.length()/BLOCK_SIZE;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < BLOCK_SIZE/Size; j++) {
			std::string tmps = bitplain.substr(i*BLOCK_SIZE+j*Size,Size);
			tmp[j].write(BIT(tmps));
		}
		/* Step Mappings */ 
		Round(h,tmp); 
	}
	/* Output */
	for(int i=0;i<8;++i){
		unsigned long l=(h[i].read()).to_ulong();
		printf("%08x",(unsigned int)l);
	}	
}

int main()
{
	freopen("plain.txt", "r", stdin);
	freopen("SHA3-512-cipher.txt", "w", stdout);
	
	Initializer();
	Plaintext_Input();
	SHA2();
	puts(""); 
	Printinfo(stdout);
	
	return 0;
	
} 

