#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

#include "plusaes.hpp" // uses the header only plusaes AES decryption/encryption
#include "zstd.h" // facebook/zsdt

unsigned char byte;
unsigned long int fileSize;
const int headerSize = 32;
std::string filePath;
unsigned int gameChoice;

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
	},
	{"Tokyo Necro", {
		0x92, 0x0A, 0x2C, 0xBD, 0x4A, 0xF0, 0x19, 0xC9,
		0x5F, 0x4E, 0x94, 0x2D, 0x05, 0xF9, 0x06, 0xC7,
		0xA6, 0x81, 0x26, 0xCD, 0x85, 0x84, 0x6E, 0x5A,
		0x66, 0x92, 0xC7, 0xCA, 0x04, 0x83, 0xD1, 0x85
	}
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
	std::vector<unsigned char> bigFile;

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

		} else if (type == 1) {
			entryDataBuffer.resize(fileArr.size());
			std::cout << "Decrypting... ";
			plusaes::decrypt_cbc(&fileArr[0], (long)fileArr.size(), &games[game].key[0], (long)32, &iv, &entryDataBuffer[0], (long)fileArr.size(), &padded_size);
			entryDataBuffer.resize(entryDataBuffer.size() - padded_size);

			if (entryDataBuffer[0] == 40 && entryDataBuffer[1] == 181 && entryDataBuffer[2] == 47 && entryDataBuffer[3] == 253) {
				std::cout << " Decompressing... ";
				entryDataBuffer = decompress(entryDataBuffer);
			}

			for (unsigned int i = 0; i < entryDataBuffer.size(); i++) {
				bigFile.push_back(entryDataBuffer[i]);
			}
			entryDataBuffer.clear();
			fileArr.clear();
		}
	}

	void writeFileFunc(std::string path, std::string fileName, std::vector<unsigned char> fileData) {
		if (!std::filesystem::exists(path + fileName.substr(0, fileName.rfind("\\") + 1))) {
			std::filesystem::create_directories(path + fileName.substr(0, fileName.rfind("\\") + 1));
		}
		writeFile.open(path + fileName, std::ios::binary);
		for (unsigned int i = 0; i < fileData.size(); i++) {
			writeFile << fileData[i];
		}
		writeFile.close();
	}

	void getEntries(std::vector<unsigned char> &entries) {
		long int nextEntryOffset = 0;
		int entryIncrementor = 0;
		int sectionAmount = 0;
		int sectionFileAmount = 0;
		
		std::vector<unsigned char> tempEntryData;


		for (unsigned int i = 0; i < entryNumberDec; i++) {
			entryIncrementor = i;
			sectionAmount = 0;

			unsigned int fLen = (unsigned int)entries[nextEntryOffset + 1];
			unsigned short sectionSizeDec;

			std::vector<unsigned char> tempSecSize(entries.begin() + nextEntryOffset + 39 + fLen, entries.begin() + nextEntryOffset + 41 + fLen);
			std::memcpy(&sectionSizeDec, tempSecSize.data(), tempSecSize.size());


			for (unsigned int i = nextEntryOffset; i < fLen + nextEntryOffset + 63  + (sectionSizeDec * 20); i++) {
				tempEntryData.push_back(entries[i]);
			}

			NPKentry entry{
				std::vector<unsigned char>(tempEntryData.begin(), tempEntryData.begin() + 3),
				std::string(tempEntryData.begin() + 3, tempEntryData.begin() + 3 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 3 + fLen, tempEntryData.begin() + 7 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 7 + fLen, tempEntryData.begin() + 39 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 39 + fLen, tempEntryData.begin() + 41 + fLen)
			};

			if (sectionSizeDec > 1) {
				std::cout << "Section size over 1, enabling file sectioning... ";
				sectionFileAmount++;
			}

			for (unsigned int i = 0; i < sectionSizeDec; i++) {

				NPKentry::segmentData segment = {
					std::vector<unsigned char>(tempEntryData.begin() + 43 + fLen + (sectionAmount * 20), tempEntryData.begin() + 51 + fLen + (sectionAmount * 20)),
					std::vector<unsigned char>(tempEntryData.begin() + 51 + fLen + (sectionAmount * 20), tempEntryData.begin() + 55 + fLen + (sectionAmount * 20)),
					std::vector<unsigned char>(tempEntryData.begin() + 55 + fLen + (sectionAmount * 20), tempEntryData.begin() + 57 + fLen + (sectionAmount * 20))
				};
				unsigned long long startOffsetDec;
				unsigned long endOffsetDec;

				std::memcpy(&startOffsetDec, segment.offset.data(), segment.offset.size());
				std::memcpy(&endOffsetDec, segment.alignedSize.data(), segment.alignedSize.size());

				//std::cout << startOffsetDec << " " << endOffsetDec << " " << sectionAmount * 20 << " next ";

				decrypt(readFile, gameChoice, startOffsetDec, endOffsetDec, 1, entry.fileName);

				sectionAmount++;
				
			}
			nextEntryOffset = nextEntryOffset + fLen + 43 + sectionAmount * 20;
			std::cout << "Written entry " << entryIncrementor + 1 << " | " << entryNumberDec << ": " << entry.fileName <<"\n";

			std::string absoluteP = filePath.substr(0, filePath.rfind("\\") + 1);
			std::replace(entry.fileName.begin(), entry.fileName.end(), '/', '\\');
			writeFileFunc(absoluteP, entry.fileName, bigFile);
			tempEntryData.clear();
			bigFile.clear();
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
	bool next = false;
	while (next == false) {

		std::cout << "Please select the game you are working with: \n";
		for (unsigned int i = 0; i < 2; i++) {
			std::cout << "\n   " << i << ":    " << games[i].name << "\n";
		}

		std::cout << "\n";
		std::cin >> gameChoice;
		if (gameChoice < 2) {
			next = true;
		} else {
			std::cout << "\nNot a valid game choice!\n";
			system("pause");
			system("cls");
		}
	}
	while (std::filesystem::path(filePath).extension().string() != ".npk") {
		std::cout << "\nPlease write/drop the path of your NPK file: ";
		std::getline(std::cin, filePath);

		if (!std::filesystem::exists(filePath)) {
			std::cout << "\nYour file path is not valid/doesn't exist!";
		}
		else if (std::filesystem::path(filePath).extension().string() != ".npk") {
			std::cout << "\nWritten/dropped file is not an NPK!";
		}
	}
	
	NPKmanip NPK(filePath);

	NPK.readHeader();
	std::string null = "Null";
	NPK.decrypt(NPK.readFile, gameChoice, headerSize, NPK.dataOffsetDec, 0, null);
	std::cout << "\nEntry array decrypted... Starting file entries decryption.\n";
	NPK.getEntries(NPK.entryBuffer);



}
