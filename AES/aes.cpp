#include<iostream>
#include<fstream>
#include<string>


#define BLOCKBYTE 16
//0x00~0xFF
#define RANGE 256

using namespace std;

// GF(2^8) of multiplication
unsigned char mul[256][256];
unsigned char sbox[256];
void makeSbox();
void makeMulGF();
// print hex value
void printBlock(char* b);

// RoundKey (Key Expand)


//Substitute Byte

//Shift Rows

//Mix Columns

//Round Key

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

	/// encrypt
	cipherFile.write(cipherText, BLOCKBYTE);
	
	// decrypt
	decryptFile.write(decrypted, BLOCKBYTE);

	//close filestreams
	plainFile.close();
	keyFile.close();
	cipherFile.close();
	decryptFile.close();
}

void makeMulGF()
{
	//
	for (int i = 0; i < 256; i++)
	{
		
	}
}

void makeSbox()
{
	
}

void printBlock(char* b)
{
	unsigned char* c = (unsigned char*)b;
	for (int i = 0; i < 16; i++)
	{
		if (c[i] < 10)
		{
			printf("0%x ", c[i]);
			continue;
		}
		printf("%x ", c[i]);
	}
	printf("\n");
}