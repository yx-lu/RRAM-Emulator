#include <iostream>
#include "rram_allocator.h"

const int ROUND_NUM = 24;
const int BLOCK_SIZE = 576;
const int OUTPUT_LENGTH = 512;
const int WIDTH = 1600;
const int MAXLEN = BLOCK_SIZE*64;
const int roffset[5][5] = 
{ { 0,36, 3,41,18},
  { 1,44,10,45, 2},
  {62, 6,43,15,61},
  {28,55,25,21,56},
  {27,20,39, 8,14}
};

const BIT RC[ROUND_NUM] = {
	0x0000000000000001ULL,0x0000000000008082ULL,
	0x800000000000808AULL,0x8000000080008000ULL,
	0x000000000000808BULL,0x0000000080000001ULL,
	0x8000000080008081ULL,0x8000000000008009ULL,
	0x000000000000008AULL,0x0000000000000088ULL,
	0x0000000080008009ULL,0x000000008000000AULL,
	0x000000008000808BULL,0x800000000000008BULL,
	0x8000000000008089ULL,0x8000000000008003ULL,
	0x8000000000008002ULL,0x8000000000000080ULL,
	0x000000000000800AULL,0x800000008000000AULL,
	0x8000000080008081ULL,0x8000000000008080ULL,
	0x0000000080000001ULL,0x8000000080008008ULL
};
row rRC[ROUND_NUM];

struct StateArray {row lane[5][5];};
typedef BIT State[25];


BIT shifter_bit[Size];
RRAM shifter;
void Initializer() {
	shifter_bit[0].set(Size-1);
	for (int i = 1; i < Size; i++) shifter_bit[i].set(i-1);
	shifter = RRAM(shifter_bit);
	
	for (int i = 0; i < ROUND_NUM; i++) rRC[i].write(RC[i]);
}
void shift(int d,BIT &bits) {
	while (d--) {
		shifter.write_buf(bits);
		shifter.mult();
		bits = shifter.read_buf();
	}
}

int len;
char plain[MAXLEN];
std::string bitplain;
void Plaintext_Input() {
	while (len<MAXLEN && scanf("%c",&plain[len++])!=EOF);
	if (len==MAXLEN) {
		std::cerr << "Error: Input too long. Maximum Length: "
				  << MAXLEN << std::endl;
		exit(0);
	}	
}
void Padding() {
	bitplain = "";
	for (int i = 0; i < len/(Size/8); i++) {
		unsigned long long tmp = 0;
		for (int j = (i+1)*(Size/8)-1; j >= i*(Size/8); j--) {
			tmp<<=8; tmp|=plain[j];
		}
		BIT tmpbit(tmp);
		bitplain += tmpbit.to_string();
	}
	bitplain += "01";
	int delta = ((-(int)bitplain.length()-2)%BLOCK_SIZE+BLOCK_SIZE)%BLOCK_SIZE;
	bitplain += "1";
	bitplain += std::string(delta,'0');
	bitplain += "1";
}

void Round(StateArray &A, int round_num) {
	std::cout<<round_num<<std::endl;
	row B[5][5];
	row C[5];
	row D[5];
	/* theta step */
	for (int x = 0; x < 5; x++) 
		for (int y = 0; y < 5; y++) C[x] ^= A.lane[x][y];
	for (int x = 0; x < 5; x++) {
		BIT cshift = C[(x+1)%5].read();
		shift(1, cshift);
		row tmp; tmp.write(cshift);
		D[x] = deepcopy(C[(x+4)%5]);
		D[x] ^= tmp;
	}
	for (int x = 0; x < 5; x++) 
		for (int y = 0; y < 5; y++) A.lane[x][y]^=D[x];
	/* rho and pi steps */
	for (int x = 0; x < 5; x++)
		for (int y = 0; y < 5; y++) {
			BIT ashift = A.lane[x][y].read(); 
			shift(roffset[x][y], ashift);
			B[y][(2*x+3*y)%5].write(ashift);
		}
	/* chi step */
	for (int x = 0; x < 5; x++)
		for (int y = 0; y < 5; y++) {
			A.lane[x][y] = deepcopy(B[(x+2)%5][y]);
			A.lane[x][y]&=!B[(x+1)%5][y];
			A.lane[x][y]^=B[x][y];
		}
	/* iota step */
	A.lane[0][0]^=rRC[round_num];
}

void Keccak_f_1600(State &S) {
	/* State -> StateArray */
	StateArray A;
	for (int x = 0; x < 5; x++) 
		for (int y = 0; y < 5; y++)
			A.lane[x][y].write(S[5*y+x]);
	/*for (int y = 0; y < 5; y++) 
		for (int x = 0; x < 5; x++) {
			std::cout<<x<<" "<<y<<" "<<A.lane[x][y].read().to_string()<<std::endl;
		}*/
	/* Step Mappings */
	for (int i = 0; i < ROUND_NUM; i++) Round(A, i);
	/* StateArray -> State */
	for (int x = 0; x < 5; x++) 
		for (int y = 0; y < 5; y++)
			S[5*y+x] = A.lane[x][y].read();
}

void output(const BIT &bits) {
	unsigned long long tmp = bits.to_ullong();
	for (int i = 0; i < Size/8; i++) {
		printf("%02x",(unsigned int)(tmp&0xFF));
		tmp>>=8;
	}
}

void Sponge() {
	Padding();
	int n = bitplain.length()/BLOCK_SIZE;
	State S;
	for (int i = 0; i < n; i++) {
		printf("%d\n",i);
		for (int j = 0; j < BLOCK_SIZE/Size; j++) {
			BIT now(bitplain.substr(i*BLOCK_SIZE+j*Size,Size));
			S[j]^=now;
		}
		Keccak_f_1600(S);
	}
	
	/* Output */
	for (int i = 0; i < OUTPUT_LENGTH / Size; i++) output(S[i]); puts("");
}

int main()
{
	freopen("plain.txt", "r", stdin);
	//freopen("SHA3-512-cipher.txt", "w", stdout);
	
	Initializer();
	Plaintext_Input();
	Sponge();
	//Printinfo(stdout);
	
	return 0;
	
} 
