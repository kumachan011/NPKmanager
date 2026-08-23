#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

#include "plusaes.hpp"

unsigned char byte;
std::string filePath;

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

struct NPKentry {
public:
	unsigned long entryOffset = NULL;
	unsigned int int_fileNameLength = NULL;

	unsigned char fileNameLength[2] = {};

	std::vector<unsigned char> fileName;
	NPKentry() {
		int_fileNameLength = (unsigned int)fileNameLength[0] + (unsigned int)fileNameLength[1] + (unsigned int)fileNameLength[2];
	}

	unsigned char realSize[4] = {};

	unsigned char SHA256[32] = {}; // not important for now

	unsigned char sectionSize[4] = {};

	struct segmentData {
		unsigned char offset[8] = {};

		unsigned char alignedSize[4] = {};

		unsigned char compressedSize[4] = {};
	};

};


class NPKmanip // all the file manipulation functions
{


private: //file header container
	std::vector<unsigned char> NPKheader;
	std::vector <unsigned char> dataBuffer;
	std::string absoluteFilePath;

	unsigned char NPKver[8] = {}; // not needed yet
	unsigned char iv[16] = {};
	unsigned char entryNumber[4] = {};
	unsigned char dataOffset[4] = {};

public:
	NPKmanip(const std::string& filename) {
		readFile.open(filename, std::ios::binary);
	}
	std::string file;
	unsigned int dataOffsetDec = NULL;
	unsigned int entryNumberDec = NULL;

	std::ifstream readFile;
	std::ofstream writeFile;

public:
	void readHeader()
	{
		int headerSize = 32;
		while (readFile >> std::noskipws >> byte && headerSize > 0) {
			NPKheader.push_back(byte);
			//std::cout << byte << " ";
			headerSize--;
		}
		for (int i = 8; i < 24; i++) { //will be improved eventually
			iv[i - 8] = NPKheader[i];
		}
		for (int i = 24; i < 28; i++) {
			entryNumber[i - 24] = NPKheader[i];
		}
		std::memcpy(&entryNumberDec, entryNumber, 4);
		for (int i = 28; i < 32; i++) {
			dataOffset[i - 28] = NPKheader[i];
		}
		std::memcpy(&dataOffsetDec, dataOffset, 4);
	}

	void decrypt(std::ifstream &file, int game, unsigned int startOffset, unsigned int endOffset) {
		std::vector<unsigned char> fileArr;
		readFile.seekg(startOffset);
		for (unsigned int i = 0; i < endOffset; i++) {
			readFile >> byte;
			fileArr.push_back(byte);
		}
		std::vector<unsigned char> dataBuffer(fileArr.size());
		plusaes::decrypt_cbc(&fileArr[0], (long)fileArr.size(), &games[game].key[0], (long)32, &iv, &dataBuffer[0], (long)dataBuffer.size(), NULL);
		std::cout << "File decrypted.\n";
		writeFile.open(filePath + "~", std::ios::binary); // will be removed eventually
		for (unsigned int i = 0; i < dataBuffer.size(); i++) {
			writeFile << dataBuffer[i];
		}
		std::cout << "File finished writing at path >> " << filePath + "~";
	}

	//std::vector<unsigned char> &entries
	void getEntries(std::vector<unsigned char> &entries) {
		long int lastEntryOffset;
		long int newEntryOffset;
		for (unsigned int i = 0; i < entryNumberDec; i++) {
			std::cout << entryNumberDec << " i" << i << " ";
			NPKentry entry[]{
				{}
			};
			NPKentry::segmentData segmentData[]{
				{}
			}; // the program will fetch the newEntryOffset by calculating everything in the NPKentry + the last entry offset, and itll make
			// an array of all entry data, then it will call the void decrypt() function in order to decrypt all those entries. furthermore, if needed
			// they will be decompressed in void decompress()
		}
	}

	void decompress() {

	}
};

int main()
{

	while (!std::filesystem::exists(filePath)) {
		std::cout << "\nPlease insert the path of your NPK file: ";
		std::getline(std::cin, filePath);
		if (!std::filesystem::exists(filePath)) {
			std::cout << "\nYour file path is not valid/doesn't exist! \n";
		}
	}
	
	NPKmanip NPK(filePath);
	NPK.file = filePath;


	NPK.readHeader();
	NPK.decrypt(NPK.readFile, 0, 32, NPK.dataOffsetDec);



}
