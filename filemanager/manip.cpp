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
	std::vector<unsigned char> fileNameLength;

	std::vector<unsigned char> fileName;

	std::vector<unsigned char> realSize;

	std::vector<unsigned char> SHA256;// not important for now

	std::vector<unsigned char> sectionSize;

	struct segmentData {
		std::vector<unsigned char> offset;

		std::vector<unsigned char> alignedSize;

		std::vector<unsigned char> compressedSize;
	};

};


class NPKmanip // all the file manipulation functions
{


private: //file header container
	std::vector<unsigned char> NPKheader;
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


	std::vector <unsigned char> dataBuffer;
	std::vector<unsigned char> entryBuffer;


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

	void decrypt(std::ifstream &file, int game, unsigned int startOffset, unsigned int endOffset, std::string path, int type) {

		entryBuffer.clear();
		

		std::vector<unsigned char> fileArr;
		readFile.seekg(startOffset);
		for (unsigned int i = 0; i < endOffset; i++) {
			//std::cout << (unsigned int)byte;
			readFile >> byte;
			fileArr.push_back(byte);
		}
		std::cout << "done";
		dataBuffer.resize(fileArr.size());
		std::cout << "starting decryption";
		plusaes::decrypt_cbc(&fileArr[0], (long)fileArr.size(), &games[game].key[0], (long)32, &iv, &dataBuffer[0], (long)dataBuffer.size(), NULL);

		std::cout << "ended decryption";

		std::filesystem::path p(filePath);
		p = p.parent_path();
		std::string absoluteFilePath = p.string();

		writeFile.open("C:\\Users\\Kuma\\Desktop\\fun\\fun.npk", std::ios::binary); // will be removed eventually
		if (type == 0) {
			for (unsigned int i = 0; i < dataBuffer.size(); i++) {
				writeFile << dataBuffer[i];
				//std::cout << (unsigned int)dataBuffer[i] << " ";
			}
		}
		else {
			for (unsigned int i = 0; i < dataBuffer.size(); i++) {
				writeFile << entryBuffer[i];
				//std::cout << (unsigned int)dataBuffer[i] << " ";
			}
		}
		
		std::cout << "File finished writing at path >> " << filePath + "~";
		writeFile.close();
	}

	//std::vector<unsigned char> &entries
	void getEntries(std::vector<unsigned char> &entries) {
		long int nextEntryOffset = 0;
		int sectionAmount = 0;
		int sectionFileAmount = 0;

		for (unsigned int i = 0; i < entryNumberDec; i++) {
			int entryIncrementor = i;
			sectionAmount = 0;

			std::cout << "\nArchived entry: ";

			std::vector<unsigned char> tempEntryData;
			unsigned int fLen = (unsigned int)entries[nextEntryOffset + 1];

			for (int i = nextEntryOffset; i < fLen + nextEntryOffset + 63; i++) {
				tempEntryData.push_back(entries[i]);
				//std::cout << (unsigned int)tempEntryData[i] << " ";
			}
			NPKentry entry{
				std::vector<unsigned char>(tempEntryData.begin(), tempEntryData.begin() + 3),
				std::vector<unsigned char>(tempEntryData.begin() + 3, tempEntryData.begin() + 3 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 3 + fLen, tempEntryData.begin() + 7 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 7 + fLen, tempEntryData.begin() + 39 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 39 + fLen, tempEntryData.begin() + 41 + fLen)
			};

			for (int i = 0; i < entry.fileName.size(); i++) {
				std::cout << entry.fileName[i];
			}

			if ((unsigned int)entry.sectionSize[0] > 1) {
				std::cout << "\nSection size over 1, enabling file sectioning...";
				for (int i = 0; i < (unsigned int)entry.sectionSize[0]; i++) {
					NPKentry::segmentData segment[]{
						std::vector<unsigned char>(tempEntryData.begin() + 43 + fLen, tempEntryData.begin() + 51 + fLen),
						std::vector<unsigned char>(tempEntryData.begin() + 51 + fLen, tempEntryData.begin() + 55 + fLen),
						std::vector<unsigned char>(tempEntryData.begin() + 55 + fLen, tempEntryData.begin() + 57 + fLen)
					};
					/*unsigned long startOffsetDec;
					unsigned long endOffsetDec;
					std::memcpy(&startOffsetDec, &segment->offset, 8);
					std::memcpy(&endOffsetDec, &segment->alignedSize, 4);
					std::cout << startOffsetDec << " " << endOffsetDec;
					decrypt(readFile, 0, startOffsetDec, endOffsetDec, std::string(entry.fileName.begin(), entry.fileName.end()));*/
					
					//decrypt(NPK.readFile, )
					sectionAmount++;
					sectionFileAmount++;
				}
			} else if ((unsigned int)entry.sectionSize[0] == 1) {
				NPKentry::segmentData segment{
					std::vector<unsigned char>(tempEntryData.begin() + 43 + fLen, tempEntryData.begin() + 51 + fLen),
					std::vector<unsigned char>(tempEntryData.begin() + 51 + fLen, tempEntryData.begin() + 55 + fLen),
					std::vector<unsigned char>(tempEntryData.begin() + 55 + fLen, tempEntryData.begin() + 57 + fLen)
				};
				for (int i = 0; i < segment.alignedSize.size(); i++) {
					std::cout << (unsigned int)segment.alignedSize[i] << " ";
				}
				unsigned long startOffsetDec;
				unsigned long endOffsetDec;
				std::memcpy(&startOffsetDec, segment.offset.data(), 8);
				std::memcpy(&endOffsetDec, segment.alignedSize.data(), 4);
				std::cout << startOffsetDec << " " << endOffsetDec + startOffsetDec;
				decrypt(readFile, 0, startOffsetDec, endOffsetDec + startOffsetDec, std::string(entry.fileName.begin(), entry.fileName.end()), 1);
				sectionAmount = 1;
			}

			nextEntryOffset = nextEntryOffset + fLen + 43 + sectionAmount * 20;
			
			 // the program will fetch the nextEntryOffset by calculating everything in the NPKentry + the last entry offset, and itll make
			// an array of all entry data, then it will call the void decrypt() function in order to decrypt all those entries. furthermore, if needed
			// they will be decompressed in void decompress()
		}

		std::cout << "\nDecrypted file had " << entryNumberDec << " entries, with a total of " << sectionFileAmount << " files over 64kb.\n";
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
	NPK.decrypt(NPK.readFile, 0, 32, NPK.dataOffsetDec, "C:\\Users\\Kuma\\Desktop\\totono\\You and Me and Her", 0);
	NPK.getEntries(NPK.dataBuffer);



}
