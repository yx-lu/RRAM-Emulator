#include <iostream>
#include <algorithm>
#include "operation.cpp"

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
const int S2Ax[25] = {
	0, 1, 2, 3, 4,
	0, 1, 2, 3, 4,
	0, 1, 2, 3, 4,
	0, 1, 2, 3, 4,
	0, 1, 2, 3, 4
};
const int S2Ay[25] = {
	0, 0, 0, 0, 0,
	1, 1, 1, 1, 1,
	2, 2, 2, 2, 2,
	3, 3, 3, 3, 3,
	4, 4, 4, 4, 4
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

void output(const BIT &bits) {
	unsigned long long tmp = bits.to_ullong();
	for (int i = 0; i < Size/8; i++) {
		printf("%02x",(unsigned int)(tmp&0xFF));
		tmp>>=8;
	}
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
		std::string tmp = std::bitset<8>((unsigned)plain[i]).to_string();
		reverse(tmp.begin(), tmp.end());
		bitplain += tmp;
	}
	bitplain += "01";
	int delta = ((-(int)bitplain.length()-2)%BLOCK_SIZE+BLOCK_SIZE)%BLOCK_SIZE;
	bitplain += "1";
	bitplain += std::string(delta,'0');
	bitplain += "1";
}

row B[5][5];
row C[5];
row D[5];
row tmp; 
void Round(StateArray &A, int round_num) {
	/* theta step */
	for (int x = 0; x < 5; x++) C[x].reset();
	for (int x = 0; x < 5; x++) 
		for (int y = 0; y < 5; y++) C[x] ^= A.lane[x][y];
	for (int x = 0; x < 5; x++) {
		shift(1,C[(x+1)%5],tmp);
		D[x].set();
		D[x]&=C[(x+4)%5];
		D[x] ^= tmp;
	}
	for (int x = 0; x < 5; x++) 
		for (int y = 0; y < 5; y++) A.lane[x][y]^=D[x];
	/* rho and pi steps */
	for (int x = 0; x < 5; x++)
		for (int y = 0; y < 5; y++) 
			shift(roffset[x][y], A.lane[x][y], B[y][(2*x+3*y)%5]);
	/* chi step */
	for (int x = 0; x < 5; x++)
		for (int y = 0; y < 5; y++) {
			A.lane[x][y].set();
			A.lane[x][y]&=B[(x+2)%5][y];
			A.lane[x][y]&=!B[(x+1)%5][y];
			A.lane[x][y]^=B[x][y];
		}
	/* iota step */
	A.lane[0][0]^=rRC[round_num];
}

void Sponge_with_Keccak_f_1600() {
	Padding();
	StateArray A;
	int n = bitplain.length()/BLOCK_SIZE;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < BLOCK_SIZE/Size; j++) {
			std::string tmps = bitplain.substr(i*BLOCK_SIZE+j*Size,Size);
			reverse(tmps.begin(),tmps.end());
			tmp.write(BIT(tmps));
			A.lane[S2Ax[j]][S2Ay[j]]^=tmp;
		}
		/* Step Mappings */ 
		for (int itr = 0; itr < ROUND_NUM; itr++) Round(A, itr); 
	}
	
	/* Output */
	for (int i = 0; i < OUTPUT_LENGTH / Size; i++) output(A.lane[S2Ax[i]][S2Ay[i]].read()); puts("");
}

int main()
{
	freopen("plain.txt", "r", stdin);
	freopen("SHA3-512-cipher.txt", "w", stdout);
	
	Initializer();
	Plaintext_Input();
	Sponge_with_Keccak_f_1600(); 
	puts(""); 
	Printinfo(stdout);
	
	return 0;
	
} 

