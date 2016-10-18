#include<iostream>
#include<fstream>
#include<string>

#define BLOCKSIZE 16

using namespace std;

// print hex value
void printByte(char* b);

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

	char plainText[BLOCKSIZE + 2];
	char key[BLOCKSIZE + 2];
	char cipherText[BLOCKSIZE + 2];
	char decrypted[BLOCKSIZE + 2];
	plainFile.read(plainText, BLOCKSIZE);
	keyFile.read(key, BLOCKSIZE);

	cout << "PLAIN\t: ";
	printByte(plainText);
	cout << "KEY\t: ";
	printByte(key);

	/// encrypt
	cipherFile.write(plainText, BLOCKSIZE);
	
	// decrypt
	decryptFile.write(plainText, BLOCKSIZE);

	//close filestreams
	plainFile.close();
	keyFile.close();
	cipherFile.close();
	decryptFile.close();
}

void printByte(char* b)
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