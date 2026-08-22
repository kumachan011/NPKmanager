#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "plusaes.hpp"

char byte;

struct NPKgame {
	std::string name;
	const std::vector<unsigned char> key;
};

NPKgame games[] = {
	{"You and Me and Her: a Love Story", {
		0xE7, 0xE8, 0xA5, 0xF9, 0x9B, 0xAF, 0x7C, 0x73,
		0xAE, 0x6B, 0xDF, 0x3D, 0x8C, 0x90, 0x26, 0x2F,
		0xF2, 0x50, 0x25, 0xA1, 0x2D, 0xB5, 0x39, 0xF9,
		0xCF, 0xD6, 0xE8, 0xE5, 0x79, 0x75, 0xB7, 0x98 }
	}
	// more to be added when game key is requested/found
};

class NPKentry {
public:
	
};

class NPKmanip // all the file manipulation functions
{
std::ifstream readFile;


private: //file header
	std::vector<unsigned char> NPKheader;

	unsigned char NPKver[8] = {}; // not needed yet
	unsigned char iv[16] = {};
	unsigned char entryNumber[4] = {}; //not sure, not needed yet
	unsigned char dataOffset[4] = {};

public:
	NPKmanip(const std::string& filename) {
		readFile.open(filename, std::ios::binary);
	}
	std::string file;

	void readHeader()
	{
		int headerSize = 32;
		while (readFile >> byte && headerSize >= 0) {
			NPKheader.push_back(byte);
			headerSize--;
		}
		for (int i = 8; i < 24; i++) {
			iv[i - 8] = NPKheader[i];
		}
		for (int i = 32; i > 28; i--) {
			dataOffset[i - 28] = NPKheader[i];
		}
	}

	void decrypt(std::ifstream &file, int game, unsigned int startOffset, unsigned int endOffset) {
		std::vector<unsigned int> fileArr;
		readFile.seekg(startOffset);
		for (int i = startOffset; i < endOffset; i++) {
			readFile >> byte;
			fileArr.push_back(byte);
		}

		//plusaes::decrypt_cbc(&fileArr[0], fileArr.size(), &games[game].key[0], games[game].key.size(), );
	}

	void getEntries() {

	}

	void decompress() {

	}
};

int main()
{


	std::cout << "Please insert the path of your NPK file: ";
	std::string filePath;
	std::getline(std::cin, filePath);

	if (!std::filesystem::exists(filePath)) {
		std::cout << "Your file path is not valid/doesn't exist! \n";
		exit(1);
	}
	
	NPKmanip NPK(filePath);
	NPK.file = filePath;
	std::cout << NPK.file;


	NPK.readHeader();
	


	//NPK.decrypt();


}
