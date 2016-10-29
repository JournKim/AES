#include<iostream>
#include<fstream>
#include<string>


#define BLOCKBYTE 16


// Hex Expresstion of Irreducible Polynomials
//#define IPOLY 0x11B
#define IPOLY 0x165

using namespace std;

// arrays for Encrypt, Decrypt
int sbox[16][16];
int invSbox[16][16];
int RKey[44][4];// = { 0, };
int RC[10];

// print hex value
void printText(char* b);
void printBlock(int** block);

// Key Expansion
void makeRC();
void keyExpansion(char* key);

// Galois Field Operations
int add(int a, int b);
int multi(int a, int b);
int inverse(int a);

// for Extended Euclidean Algorithm
int high(int a);
int divide(int a, int b);

//Substitute Byte
void makeSbox();
int getSBox(int c);
int getInvSBox(int c);
int sX(int c);

void subByte(int** block);
void invSubByte(int** block);

//Shift Rows
void shiftRows(int** block);
void invShiftRows(int** block);

//Mix Columns
void mixColumns(int** block);
void invMixColumns(int** block);

//Round Key
void addRoundKey(int** block, int round);

//Encryption, Decryption
void textToBlock(char* pT, int** block);
void blockToText(int** block, char* cT);
void encrypt(char* pT, char* cT);
void decrypt(char* cT, char* pT);

int main()
{
	//declare fstreams and open files
	ifstream plainFile;
	ifstream keyFile;
	ofstream cipherFile;
	ofstream decryptFile;
	plainFile.open("plain.bin",ios_base::in);
	keyFile.open("key.bin",ios_base::in);
	cipherFile.open("cipher.bin", ios_base::out);
	decryptFile.open("decrypt.bin", ios_base::out);
	
	if (!plainFile.is_open() || !keyFile.is_open() 
		|| !cipherFile.is_open() || !decryptFile.is_open()) {
		cout << "File Not Opened" << endl;
		return -1;
	}
	// arrays to store texts
	char plainText[BLOCKBYTE + 2];
	char key[BLOCKBYTE + 2];
	char cipherText[BLOCKBYTE + 2];
	char decrypted[BLOCKBYTE + 2];

	// read plaintext, key
	plainFile.read(plainText, BLOCKBYTE);
	keyFile.read(key, BLOCKBYTE);

	cout << "PLAIN\t: ";
	printText(plainText);
	cout << "KEY\t: ";
	printText(key);
	cout << endl;
	
	//make RC, SBox;
	makeRC();
	makeSbox();
	
	// encryption Start
	cout << "<------ ENCRYPTION ------>\n\n";

	// Key Expand
	cout << "KEY EXPANSION" << endl;
	keyExpansion(key);
	cout << endl;

	encrypt(plainText, cipherText);
	// encryption End

	// write ciphertext to file
	cipherFile.write(cipherText, BLOCKBYTE);
	
	// decryption Start
	cout << "\n<------ DECRYPTION ------>\n\n";
	decrypt(cipherText, decrypted);
	// decryption End

	// write decrypted text to file
	decryptFile.write(decrypted, BLOCKBYTE);

	//close filestreams
	plainFile.close();
	keyFile.close();
	cipherFile.close();
	decryptFile.close();
}

// Substitute Bytes
void subByte(int ** block)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			// get SBox value, put it into block
			block[i][j] = getSBox(block[i][j]);
		}
	}
	cout << "SB : ";
	printBlock(block);
}

// Inverse Substitute Bytes
void invSubByte(int ** block)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			// get InvSBox value, put it into block
			block[i][j] = getInvSBox(block[i][j]);
		}
	}
	cout << "SB : ";
	printBlock(block);
}

// Shift Rows
void shiftRows(int ** block)
{
	int t;
	for (int i = 1; i < 4; i++)
	{
		// left circular shift by row number
		for (int j = 0; j < i; j++)
		{
			t = block[i][0];
			block[i][0] = block[i][1];
			block[i][1] = block[i][2];
			block[i][2] = block[i][3];
			block[i][3] = t;
		}
	}

	cout << "SR : ";
	printBlock(block);
}

// Inverse Shift Rows
void invShiftRows(int ** block)
{
	int t;
	for (int i = 1; i < 4; i++)
	{
		// right circular shift by row number
		for (int j = 0; j < 4-i; j++)
		{
			t = block[i][0];
			block[i][0] = block[i][1];
			block[i][1] = block[i][2];
			block[i][2] = block[i][3];
			block[i][3] = t;
		}
	}

	cout << "SR : ";
	printBlock(block);
}

// Mix Columns
void mixColumns(int ** block)
{
	// Constant matrix
	int s[4][4] = {
		{0x02, 0x03, 0x01, 0x01},
		{0x01, 0x02, 0x03, 0x01},
		{0x01, 0x01, 0x02, 0x03},
		{0x03, 0x01, 0x01, 0x02}
	};
	int r[4];

	// Matrix Multiplication
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			// ^(XOR) means addtion in Galois Field
			r[j] = multi(s[j][0], block[0][i]);
			r[j] ^= multi(s[j][1], block[1][i]);
			r[j] ^= multi(s[j][2], block[2][i]);
			r[j] ^= multi(s[j][3], block[3][i]);
		}
		block[0][i] = r[0];
		block[1][i] = r[1];
		block[2][i] = r[2];
		block[3][i] = r[3];
	}
	cout << "MC : ";
	printBlock(block);
}

// Inverse Mix Columns
void invMixColumns(int ** block)
{
	// Constant matrix
	int s[4][4] = {
		{ 0x0E, 0x0B, 0x0D, 0x09 },
		{ 0x09, 0x0E, 0x0B, 0x0D },
		{ 0x0D, 0x09, 0x0E, 0x0B },
		{ 0x0B, 0x0D, 0x09, 0x0E }
	};
	int r[4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			r[j] = multi(s[j][0], block[0][i]);
			r[j] ^= multi(s[j][1], block[1][i]);
			r[j] ^= multi(s[j][2], block[2][i]);
			r[j] ^= multi(s[j][3], block[3][i]);
		}
		block[0][i] = r[0];
		block[1][i] = r[1];
		block[2][i] = r[2];
		block[3][i] = r[3];
	}
	cout << "MC : ";
	printBlock(block);
}

// Add Round Key
void addRoundKey(int ** block, int round)
{
	// XOR(Addition in Galois Field) block and round key
	int index = round * 4;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			block[j][i] ^= (RKey[index+i][j] % 0x100);
		}
	}
	cout << "AR : ";
	printBlock(block);
}

// convert text into 4*4 matrix form
void textToBlock(char * pT, int ** block)
{
	for (int i = 0; i < 16; i++)
	{
		block[i % 4][i / 4] = 0;
		block[i % 4][i / 4] = ((unsigned char)pT[i]) % 0x100;
		block[i % 4][i / 4] %= 0x100;
	}
}

// convert 4*4 matrix to text
void blockToText(int ** block, char * cT)
{
	for (int i = 0; i < 16; i++)
	{
		cT[i] = block[i % 4][i / 4];
	}
	cT[16] = 0;
}

// encrypt plaintext to ciphertext
void encrypt(char * pT, char * cT)
{
	// 4*4 matrix to store text
	int** block = new int*[4];
	for (int i = 0; i < 4; i++)
	{
		block[i] = new int[4];
	}
	textToBlock(pT, block);

	// Round 0 to Round 10
	for (int i = 0; i <= 10; i++)
	{
		printf("Round %d\n", i);
		if (i != 0) {
			subByte(block);
			shiftRows(block);
			if (i != 10)
			{
				mixColumns(block);
			}
		}
		addRoundKey(block, i);
		printf("\n");
	}
	blockToText(block, cT);

	cout << "CIPHER : ";
	printText(cT);
}

// decrypt ciphertext to plaintext
void decrypt(char * cT, char * pT)
{
	int** block = new int*[4];
	for (int i = 0; i < 4; i++)
	{
		block[i] = new int[4];
	}
	textToBlock(cT, block);

	for (int i = 0; i <= 10; i++)
	{
		printf("Round %d\n", i);
		if (i != 0)
		{
			invShiftRows(block);
			invSubByte(block);
		}
		addRoundKey(block, 10 - i); // reverse order to encryption
		if( i!= 0 && i != 10)
		{
			invMixColumns(block);
		}
		printf("\n");
	}
	blockToText(block, pT);
	printf("DECRYPTED : ");
	printText(pT);
}

// get inverse of n in galois field
int inverse(int n) {
	
	/*for (int i = 0; i < 256; i++)
	{
		if (multi(i, n) == 1)
			return i;
	}
	return 0;*/
	
	// ****** Not Complete *******//
	// Extended Euclidean Algorithm
	int a[3] = { 1,0,IPOLY ^ 0x100 };
	int b[3] = { 0,1,n };
	int c[3];
	int q;
	while (1) {
		if (b[2] == 0)
		{
			return -1;
		}
		if (b[2] == 1)
		{
			return (int)(b[1] % 0x100);
		}
		q = divide(a[2], b[2]);
		for (int i = 0; i < 3; i++)
			c[i] = a[i] ^ multi(q,b[i]);

		for (int i = 0; i < 3; i++)
		{
			a[i] = b[i];
			b[i] = c[i];
		}
	}
}

// return polynomial's highest degree
int high(int a)
{
	int h = 0;
	while (a)
	{
		h++;
		a >>= 1;
	}
	return h;
}

// get quotient
int divide(int a, int b)
{
	int ret = 0;
	int remain = 987654321;
	if (a < b)
	{
		return 0;
	}
	int sub = high(a) - high(b);
	remain = a ^ (b << sub);
	ret += (1 << sub);

	while (high(remain) >= high(b))
	{
		sub = high(remain) - high(b);
		remain = remain ^ (b << sub);
		ret += (1 << sub);
	}
	return ret;
}

// addition in Galois Field
int add(int a, int b)
{
	return (a^b) %0x100;
}

// multiplication in Galois Field
int multi(int a, int b)
{
	int ret = 0;
	int arr[8];
	arr[0] = b;
	
	for (int i = 1; i < 8; i++)
	{
		arr[i] = arr[i - 1] << 1;
		if (arr[i] & 0x100)
			arr[i] ^= IPOLY;
		//printf("arr[%d] : %x\n", i, arr[i]);
	}
	int comp = 1;
	for (int i = 0; i < 8; i++)
	{
		if (a & comp)
			ret ^= arr[i];
		comp <<= 1;
	}
	return ret;
}

// matrix operation for make s-box
int sX(int b)
{
	// constant matrix
	int arr[8][8] = {
		{1,0,0,0,1,1,1,1},
		{1,1,0,0,0,1,1,1},
		{1,1,1,0,0,0,1,1},
		{1,1,1,1,0,0,0,1},
		{1,1,1,1,1,0,0,0},
		{0,1,1,1,1,1,0,0},
		{0,0,1,1,1,1,1,0},
		{0,0,0,1,1,1,1,1}
	};
	// constant matrix
	int aV[8] = { 1,1,0,0,0,1,1,0 };

	// bit expression of b
	int byteArr[8];
	int tmp = 0x1;
	for (int i = 0; i < 8; i++)
	{
		if (tmp & b)
			byteArr[i] = 1;
		tmp <<= 1;
	}

	// matrix operation ( arr * byteArr + aV )
	int ret[8] = { 0, };
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			ret[i] ^= (arr[i][j] & byteArr[j]);
		}
		ret[i] ^= aV[i];
	}

	// convert bit expresstion to integer
	int result = 0;
	tmp = 0x1;
	for (int i = 0; i < 8; i++)
	{
		result += (ret[i] * tmp);
		tmp <<= 1;
	}
	return result;
}


// make s-box and inverse s-box
void makeSbox()
{
	bool chk[256] = { 0, }; // for inverse of zero
	
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			int t = inverse(i*16 + j);
			t = sX(t);
			sbox[i][j] = t;
			chk[t] = 1;
			invSbox[t / 16][t % 16] = i * 16 + j;
		}
	}

	// checkout s-box value of zero
	for (int i = 0; i < 256; i++)
	{
		if (chk[i] == false)
		{
			sbox[0][0] = i;
			invSbox[i / 16][i % 16] = 0;
		}
	}
	
	return;
}

// print hex form of text
void printText(char* b)
{
	unsigned char* c = (unsigned char*)b;
	for (int i = 0; i < 16; i++)
	{
		if (c[i] < 0x10)
		{
			printf("0%X ", c[i]);
			continue;
		}
		printf("%X ", c[i]);
	}
	printf("\n");
}

// print hex form of 4*4 matrix block
void printBlock(int ** block)
{
	unsigned int **b = (unsigned int**)block;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			//if (b[j][i] < 16)
			//{
			//	//printf("0");
			//	printf("%X ", b[j][i] % 0x100);
			//	continue;
			//}
				
			printf("%2X ", ((b[j][i]) % 0x100));
		}
	}
	cout << endl;
}

// print Key Expansion Progress
void printBlockR(int i)
{
	unsigned char ch;
	for (int j = 0; j < 4; j++)
	{
		//printf("(");
		for (int k = 0; k < 4; k++)
		{
			ch = RKey[i * 4 + j][k];
			if (ch < 0x10)
			{
				printf("0%X ", ch);
				continue;
			}
			
			printf("%X ", ch);
		}
		//printf(")");
	}
	printf("\n");
}

// key expansion
void keyExpansion(char *key)
{
	// convert key to 4*4 matrix form
	for (int i = 0; i < 16; i++)
	{
		RKey[i / 4][i % 4] = key[i];
	}
	cout << "ROUND 0 : ";
	printBlockR(0);
	
	//repeat 10 ;
	for (int i = 1; i <= 10; i++)
	{
		int index = 4 * i;
		
		for (int j = 0; j < 4; j++)
		{
			RKey[index + j][0] = RKey[index + j - 4][0];
			RKey[index + j][1] = RKey[index + j - 4][1];
			RKey[index + j][2] = RKey[index + j - 4][2];
			RKey[index + j][3] = RKey[index + j - 4][3];
		}
		
		// index + 0
		{
			int tmp[4];
			tmp[0] = RKey[index - 1][1];
			tmp[1] = RKey[index - 1][2];
			tmp[2] = RKey[index - 1][3];
			tmp[3] = RKey[index - 1][0];
			for (int j = 0; j < 4; j++)
			{
				tmp[j] = getSBox(tmp[j]);
			}
			tmp[0] = tmp[0] ^ RC[i - 1];
			//cout << i << "th Sboxbox" << endl; 
			for (int j = 0; j < 4; j++)
			{
				RKey[index + j][0] = (RKey[index + j][0] ^ tmp[0]) % 0x100;
				RKey[index + j][1] = (RKey[index + j][1] ^ tmp[1]) % 0x100;
				RKey[index + j][2] = (RKey[index + j][2] ^ tmp[2]) % 0x100;
				RKey[index + j][3] = (RKey[index + j][3] ^ tmp[3]) % 0x100;

				tmp[0] = RKey[index + j][0];
				tmp[1] = RKey[index + j][1];
				tmp[2] = RKey[index + j][2];
				tmp[3] = RKey[index + j][3];
			}	
		}

		printf("ROUND %d : ", i);
		printBlockR(i);
	}
}

// return s-box value of c
int getSBox(int c)
{
	unsigned char ch = c % 0x100;
	return sbox[ch/0x10][ch%0x10];
}

// return inverse s-box value of c
int getInvSBox(int c)
{
	unsigned char ch = c % 0x100;
	return invSbox[ch / 0x10][ch % 0x10];
}

// set Round Constant
void makeRC()
{
	cout << "RC\t: ";
	RC[0] = 0x01;

	for (int i = 1; i < 10; i++)
	{
		RC[i] = RC[i - 1] * 2;
		if (RC[i] & 0x100)
			RC[i] ^= IPOLY;
	}
	for (int i = 0; i < 10; i++)
	{
		if (RC[i] < 0x10)
		{
			printf("0%X ", RC[i]);
			continue;
		}
		printf("%X ", RC[i]);
	}
	cout << endl;
}
