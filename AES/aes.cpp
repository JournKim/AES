#include<iostream>
#include<fstream>
#include<string>


#define BLOCKBYTE 16

#define IPOLY 0x165

using namespace std;
typedef int mybyte;

mybyte mul[256][256];
mybyte sbox[16][16];
mybyte invSbox[16][16];
mybyte RKey[44][4] = { 0, };
int RC[10];

// print hex value
void printBlock(char* b);
void printBlockB(char** c, int i);
// RoundKey (Key Expand)
void makeRC();
void keyExpansion(char* key);
int* RFunc(int i);

//Substitute Byte
mybyte inverse(int a);
int add(int a, int b);
int multi(int a, int b);
int high(int a);
int divide(int a, int b);
void makeSbox();
int getSBox(int c);
mybyte sX(mybyte c);
//Shift Rows

//Mix Columns

//Round Key

int main()
{	
	//cout << multi(0x16, 0x10) << endl;
	makeRC();
	//makeMulGF();
	makeSbox();
	//printf("%X\n", sX(0xE0));
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

	char plainText[BLOCKBYTE + 2];
	char key[BLOCKBYTE + 2];
	char cipherText[BLOCKBYTE + 2];
	char decrypted[BLOCKBYTE + 2];
	plainFile.read(plainText, BLOCKBYTE);
	keyFile.read(key, BLOCKBYTE);

	cout << "PLAIN\t: ";
	printBlock(plainText);
	cout << "KEY\t: ";
	printBlock(key);
	cout << endl;
	/// encrypt
	cout << "<------ ENCRYPTION ------>\n\n";

	cout << "KEY EXPANSION" << endl;
	keyExpansion(key);


	cipherFile.write(cipherText, BLOCKBYTE);
	
	// decrypt
	decryptFile.write(decrypted, BLOCKBYTE);

	//close filestreams
	plainFile.close();
	keyFile.close();
	cipherFile.close();
	decryptFile.close();
}

mybyte inverse(int n) {
	int a[3] = { 1,0,IPOLY };
	int b[3] = { 0,1,n };
	int c[3];
	//int t[3];
	int q;
	while (1) {
		if (b[2] == 0)
		{
			//printf("Inverse of %X :\t NULLLLLLLLLL\n", n);
			return -1;
		}
		if (b[2] == 1)
		{
			//printf("Inverse of %X :\t %X\n", n, b[1]);
			return (mybyte)(b[1] % 0x100);
		}
		q = divide(a[2], b[2]);
		for (int i = 0; i < 3; i++)
			c[i] = add(a[i], multi(q,b[i]));

		//printf("a : %X, b : %X, c : %X, q : %X\n",a[2],b[2],c[2],q);
		for (int i = 0; i < 3; i++)
		{
			a[i] = b[i];
			b[i] = c[i];
		}
	}
}

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

int add(int a, int b)
{
	return (a^b) %0x100;
}

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

mybyte sX(mybyte b)
{
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
	int aV[8] = { 1,1,0,0,0,1,1,0 };
	int byteArr[8];
	int tmp = 0x1;
	for (int i = 0; i < 8; i++)
	{
		if (tmp & b)
			byteArr[i] = 1;
		tmp <<= 1;
	}

	int ret[8] = { 0, };
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			ret[i] ^= (arr[i][j] & byteArr[j]);
		}
		ret[i] ^= aV[i];
	}

	mybyte result = 0;
	tmp = 0x1;
	for (int i = 0; i < 8; i++)
	{
		result += (ret[i] * tmp);
		tmp <<= 1;
	}
	return result;
}

void makeSbox()
{
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			if (multi(i, j) == 1)
			{
				sbox[i / 16][i % 16] = sX(j);
			}
		}
	}

	return;

	bool chkd[256] = { false };
	for (int r = 0; r < 16; r++)
	{
		for (int c = 0; c < 16; c++)
		{
			sbox[r][c] = inverse(r * 16 + c);
			//sX(sbox[r][c]);
		}
	}
	/*for (int i = 0; i < 256; i++)
	{
		if (chkd[i] == false)
		{
			chkd
		}
	}*/
}

void printBlock(char* b)
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

void xor4Bytes(int i, int* a, int* b)
{
	int* dest = RKey[i];

	for (int i = 0; i < 4; i++)
	{
		dest[i] = a[i] ^ b[i];
	}

	//printf("%d Word XOR\n",i);
	//printf("%X %X %X %X -> %X %X %X %X\n", a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3]);

}

void keyExpansion(char *key)
{
	for (int i = 0; i < 16; i++)
	{
		RKey[i / 4][i % 4] = key[i];
	}
	cout << "ROUND 0 : ";
	printBlockR(0);
	
	//repeat 10 ;
	for (int i = 1; i <= 10; i++)
	{
		int index = i * 4;
		int* rf = RFunc(i);

		for(int j=0; j<4; j++)
			RKey[index][j] = rf[j] ^ RKey[index - 4][j];
		for (int k = index + 1; k < index + 4; k++) {
			for (int j = 0; j < 4; j++)
			{
				RKey[k][j] = RKey[k - 1][j] ^ RKey[k - 4][j];
			}
		}

		printf("ROUND %d : ", i);
		printBlockR(i);
	}
}

int* RFunc(int i)
{
	//cout << "RFunc IN" << endl;
	int index = i * 4;
	int* c = new int[4];
	for (int i = 0; i < 4; i++)
	{
		c[i] = RKey[index-1][(i + 1) % 4];
	}

	for (int i = 0; i < 4; i++)
	{
		c[i] = getSBox(c[i]) ^ c[(i + 1) % 4];
	}
	c[0] = c[0] ^ RC[i-1];

	return c;
}

int getSBox(int c)
{
	return sbox[c/0x10][c%0x10];
}

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
