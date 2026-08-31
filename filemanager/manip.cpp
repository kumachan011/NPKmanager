#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

#include "plusaes.hpp"
#include "zstd.h"

unsigned char byte;
unsigned long int fileSize;
const int headerSize = 32;
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
	std::vector<unsigned char> fileNameLength; // first 3 bytes, if byte 1 is 01 that means the file is compressed (mostly game dialogue

	std::string fileName;// represents the length pulled from fileNameLength

	std::vector<unsigned char> realSize; // the files real size after decryption and compression, should be 4 bytes RIGHT after fileName, little endian

	std::vector<unsigned char> SHA256;// not important for now, represents 32 bytes

	std::vector<unsigned char> sectionSize; // 4 bytes right after the SHA256, 1 section represents 64 kilobytes, or 0x10000/65536 bytes

	struct segmentData { // the amount of segmentData structs is created depending on sectionSize amount
		std::vector<unsigned char> offset; // the offset of the ACTUAL data of the entry in the file

		std::vector<unsigned char> alignedSize; // how big the file is with PKCS5 padding (fancy word for adding numbers at the
		//end so the decryption block is 16 bytes)

		std::vector<unsigned char> compressedSize; // how big the file actually is without the padding (will be important for decompression if thats the case)
	};
};

NPKentry entry;
NPKentry::segmentData segment;


class NPKmanip // all the file manipulation functions
{


private: // will need to improve this
	std::vector<unsigned char> NPKheader;
	std::vector<unsigned char> fileArr;

	unsigned char NPKver[8] = {}; // not needed yet
	unsigned char iv[16] = {};
	unsigned char entryNumber[4] = {};
	unsigned char dataOffset[4] = {};

public:
	NPKmanip(const std::string& filename) {
		readFile.open(filename, std::ios::binary);
	}

	unsigned int dataOffsetDec = NULL;
	unsigned int entryNumberDec = NULL;

	std::vector <unsigned char> entryBuffer;
	std::vector<unsigned char> entryDataBuffer;

	std::ifstream readFile;
	std::ofstream writeFile;

public:
	void readHeader()
	{
		int temphHeaderSize = headerSize;
		while (readFile >> std::noskipws >> byte && temphHeaderSize > 0) {
			NPKheader.push_back(byte);
			temphHeaderSize--;
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

	void decrypt(std::ifstream &file, int game, unsigned int startOffset, unsigned int endOffset, int type, std::string fileName) {
		
		readFile.seekg(startOffset);

		std::replace(fileName.begin(), fileName.end(), '/', '\\');
		unsigned long padded_size;

		for (unsigned int i = 0; i < endOffset; i++) {
			readFile >> byte;
			fileArr.push_back(byte);
		}

		if (type == 0) {
			entryBuffer.resize(fileArr.size());
			plusaes::decrypt_cbc(&fileArr[0], (long)fileArr.size(), &games[game].key[0], (long)32, &iv, &entryBuffer[0], (long)fileArr.size(), &padded_size);

			/*writeFile.open("C:\\Users\\Kuma\\Desktop\\fun\\fun.npk", std::ios::binary);
			for (unsigned int i = 0; i < fileArr.size(); i++) {
				writeFile << entryBuffer[i];
			}*/
			//writeFile.close();
			fileArr.clear();
		}
		else if (type == 1) {
			entryDataBuffer.resize(fileArr.size());
			plusaes::decrypt_cbc(&fileArr[0], (long)fileArr.size(), &games[game].key[0], (long)32, &iv, &entryDataBuffer[0], (long)fileArr.size(), &padded_size);
			entryDataBuffer.resize(entryDataBuffer.size() - padded_size);
			std::string path = fileName.substr(0, fileName.rfind("\\") + 1);
			std::string absoluteP = filePath.substr(0, filePath.rfind("\\") + 1);

			if (!std::filesystem::exists(absoluteP + path)) {
				std::filesystem::create_directories(absoluteP + path);
			}
			writeFile.open(absoluteP + fileName, std::ios::binary);
			if (entryDataBuffer[0] == 40 && entryDataBuffer[1] == 181 && entryDataBuffer[2] == 47 && entryDataBuffer[3] == 253) {
				entryDataBuffer = decompress(entryDataBuffer);
				
			}
			for (unsigned int i = 0; i < entryDataBuffer.size(); i++) {
				writeFile << entryDataBuffer[i];
			}
			writeFile.close();
			fileArr.clear();
		}
	}

	void getEntries(std::vector<unsigned char> &entries) {
		long int nextEntryOffset = 0;
		int entryIncrementor = 0;
		int sectionAmount = 0;
		int sectionFileAmount = 0;

		unsigned long long startOffsetDec = 0;
		unsigned long endOffsetDec = 0;
		
		std::vector<unsigned char> tempEntryData;


		for (unsigned int i = 0; i < entryNumberDec; i++) {
			entryIncrementor = i;
			sectionAmount = 0;

			unsigned int fLen = (unsigned int)entries[nextEntryOffset + 1];

			for (unsigned int i = nextEntryOffset; i < fLen + nextEntryOffset + 63; i++) {
				tempEntryData.push_back(entries[i]);
			}

			NPKentry entry{
				std::vector<unsigned char>(tempEntryData.begin(), tempEntryData.begin() + 3),
				std::string(tempEntryData.begin() + 3, tempEntryData.begin() + 3 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 3 + fLen, tempEntryData.begin() + 7 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 7 + fLen, tempEntryData.begin() + 39 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 39 + fLen, tempEntryData.begin() + 41 + fLen)
			};

			if ((unsigned int)entry.sectionSize[0] > 1) {
				std::cout << "\nSection size over 1, enabling file sectioning...";
				for (unsigned int i = 0; i < (unsigned int)entry.sectionSize[0]; i++) {
					NPKentry::segmentData segment[]{
						std::vector<unsigned char>(tempEntryData.begin() + 43 + fLen, tempEntryData.begin() + 51 + fLen),
						std::vector<unsigned char>(tempEntryData.begin() + 51 + fLen, tempEntryData.begin() + 55 + fLen),
						std::vector<unsigned char>(tempEntryData.begin() + 55 + fLen, tempEntryData.begin() + 57 + fLen)
					};
					unsigned long startOffsetDec;
					unsigned long endOffsetDec;
					std::memcpy(&startOffsetDec, &segment->offset, 8);
					std::memcpy(&endOffsetDec, &segment->alignedSize, 4);


					sectionAmount++;
					sectionFileAmount++;
				}
			} else if ((unsigned int)entry.sectionSize[0] == 1) {
				NPKentry::segmentData segment{	
					std::vector<unsigned char>(tempEntryData.begin() + 43 + fLen, tempEntryData.begin() + 51 + fLen),
					std::vector<unsigned char>(tempEntryData.begin() + 51 + fLen, tempEntryData.begin() + 55 + fLen),
					std::vector<unsigned char>(tempEntryData.begin() + 55 + fLen, tempEntryData.begin() + 57 + fLen)
				};
				std::memcpy(&startOffsetDec, segment.offset.data(), segment.offset.size());
				std::memcpy(&endOffsetDec, segment.alignedSize.data(), segment.alignedSize.size());

				decrypt(readFile, 0, startOffsetDec, endOffsetDec, 1, entry.fileName);
				sectionAmount = 1;
			}

			nextEntryOffset = nextEntryOffset + fLen + 43 + sectionAmount * 20;
			std::cout << "Written entry " << entryIncrementor + 1 << " | " << entryNumberDec << ": " << entry.fileName <<"\n";
			tempEntryData.clear();
		}
		std::cout << "\nDecrypted file had " << entryNumberDec << " entries and " << sectionFileAmount << " entries with more than a single section size.\n";
	}

	std::vector<unsigned char> decompress(std::vector<unsigned char> entry) {
		unsigned long long decompressedSize =
			ZSTD_getFrameContentSize(
				entry.data(),
				entry.size() 
			);
		std::vector<unsigned char> decompressedData(decompressedSize);

		size_t const result = ZSTD_decompress(
			decompressedData.data(),
			decompressedData.size(),
			entry.data(),
			entry.size()
		);

		if (ZSTD_isError(result)) {
			std::cout << ZSTD_getErrorName(result) << '\n';
		}

		return decompressedData;
	}
};

int main()
{

	while (std::filesystem::path(filePath).extension().string() != ".npk") {
		std::cout << "\nPlease write/drop the path of your NPK file: ";
		std::getline(std::cin, filePath);

		if (!std::filesystem::exists(filePath)) {
			std::cout << "\nYour file path is not valid/doesn't exist!";
		} else if (std::filesystem::path(filePath).extension().string() != ".npk") {
			std::cout << "\nWritten/dropped file is not an NPK!";
		}
	}
	
	NPKmanip NPK(filePath);

	NPK.readHeader();
	std::cout << NPK.dataOffsetDec;
	std::string null = "Null";
	NPK.decrypt(NPK.readFile, 0, headerSize, NPK.dataOffsetDec, 0, null);
	NPK.getEntries(NPK.entryBuffer);



}
