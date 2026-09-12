#pragma once

#ifndef INFO
#define INFO
#include "information.h"
#endif


class NPKunpack // all the file unpacking functions 
{

private:
	std::vector<unsigned char> NPKheader;
	std::vector<unsigned char> fileArr;
	std::vector<unsigned char> bigFile;
	std::vector<unsigned char> entryDataBuffer;

	unsigned char iv[16] = {};
	unsigned char entryNumber[4] = {};
	unsigned char dataOffset[4] = {};

	unsigned int entryCompressedSize = 0;
	unsigned int entryDecompressedSize = 0;

	unsigned int entryNumberDec = NULL;

	inline void writeFileFunc(std::string path, std::string fileName, std::vector<unsigned char> fileData) {
		if (!std::filesystem::exists(path + fileName.substr(0, fileName.rfind("\\") + 1))) {
			std::filesystem::create_directories(path + fileName.substr(0, fileName.rfind("\\") + 1));
		}
		writeFile.open(path + fileName, std::ios::binary);
		for (unsigned int i = 0; i < fileData.size(); i++) {
			writeFile << fileData[i];
		}
		writeFile.close();
	}

public:
	NPKunpack(const std::string& filename) {
		readFile.open(filename, std::ios::binary);
	}

	unsigned int dataOffsetDec = NULL;

	std::vector <unsigned char> entryBuffer;

	std::ifstream readFile;
	std::ofstream writeFile;

public:
	inline void readHeader() {
		int temphHeaderSize = headerSize;
		while (readFile >> std::noskipws >> byte && temphHeaderSize > 0) {
			NPKheader.push_back(byte);
			temphHeaderSize--;
		}
		for (int i = 8; i < 24; i++) {
			iv[i - 8] = NPKheader[i];
		}
		for (int i = 24; i < 28; i++) {
			entryNumber[i - 24] = NPKheader[i];
		}
		for (int i = 28; i < 32; i++) {
			dataOffset[i - 28] = NPKheader[i];
		}
		std::memcpy(&entryNumberDec, entryNumber, 4);
		std::memcpy(&dataOffsetDec, dataOffset, 4);
	}

	inline void decrypt(std::ifstream &file, int game, unsigned long long startOffset, unsigned long endOffset, int type, std::string fileName) {

		readFile.seekg(startOffset);
		unsigned long padded_size;

		for (unsigned int i = 0; i < endOffset; i++) {
			readFile >> byte;
			fileArr.push_back(byte);
		}

		if (type == 0) { // decrypt entries and offsets
			entryBuffer.resize(fileArr.size());
			plusaes::decrypt_cbc(&fileArr[0], (long)fileArr.size(), &games[game].key[0], (long)32, &iv, &entryBuffer[0], (long)fileArr.size(), &padded_size);
			fileArr.clear();

		} else if (type == 1) { // decrypt entry data itself
			entryDataBuffer.resize(fileArr.size());
			plusaes::decrypt_cbc(&fileArr[0], (long)fileArr.size(), &games[game].key[0], (long)32, &iv, &entryDataBuffer[0], (long)fileArr.size(), &padded_size);
			entryDataBuffer.resize(entryDataBuffer.size() - padded_size);

			if (entryCompressedSize < entryDecompressedSize) {
				switch (games[gameChoice].NPKver) {
				case 2:
					entryDataBuffer = decompressZLIB(entryDataBuffer);
					break;
				case 3:
					entryDataBuffer = decompressZSTD(entryDataBuffer);
					break;
				}
			}

			for (unsigned int i = 0; i < entryDataBuffer.size(); i++) {
				bigFile.push_back(entryDataBuffer[i]);
			}

			entryDataBuffer.clear();
			fileArr.clear();
		}
	}

	inline void getEntries(std::vector<unsigned char> &entries) {
		long long int nextEntryOffset = 0;
		int entryIncrementor = 0;
		int sectionAmount = 0;
		int sectionFileAmount = 0;
		float filePercentage = 0;
		unsigned long long startOffsetDec = 0;
		unsigned long endOffsetDec = 0;
		unsigned int fLen = 0;
		unsigned short sectionSizeDec = 0;
		
		std::vector<unsigned char> tempEntryData;

		for (unsigned int i = 0; i < entryNumberDec; i++) {
			entryIncrementor = i;
			sectionAmount = 0;
			fLen = (unsigned int)entries[nextEntryOffset + 1];
			sectionSizeDec;

			filePercentage = (float)(((entryIncrementor + 1) / (float)(entryNumberDec)) * 100);

			std::vector<unsigned char> tempSecSize(entries.begin() + nextEntryOffset + 39 + fLen, entries.begin() + nextEntryOffset + 41 + fLen);
			std::memcpy(&sectionSizeDec, tempSecSize.data(), tempSecSize.size());
			for (long long int i = nextEntryOffset; i < fLen + nextEntryOffset + 63 + (sectionSizeDec * 20); i++) {
				tempEntryData.push_back(entries[i]);
			}

			NPKentry entry{
				false,
				std::vector<unsigned char>(tempEntryData.begin(), tempEntryData.begin() + 3),
				std::string(tempEntryData.begin() + 3, tempEntryData.begin() + 3 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 3 + fLen, tempEntryData.begin() + 7 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 7 + fLen, tempEntryData.begin() + 39 + fLen),
				std::vector<unsigned char>(tempEntryData.begin() + 39 + fLen, tempEntryData.begin() + 43 + fLen)
			};

			if (sectionSizeDec > 1) {
				sectionFileAmount++;
			}
			for (unsigned int i = 0; i < sectionSizeDec; i++) {

				NPKentry::segmentData segment = {
					std::vector<unsigned char>(tempEntryData.begin() + 43 + fLen + (sectionAmount * 20), tempEntryData.begin() + 51 + fLen + (sectionAmount * 20)),
					std::vector<unsigned char>(tempEntryData.begin() + 51 + fLen + (sectionAmount * 20), tempEntryData.begin() + 55 + fLen + (sectionAmount * 20)),
					std::vector<unsigned char>(tempEntryData.begin() + 55 + fLen + (sectionAmount * 20), tempEntryData.begin() + 59 + fLen + (sectionAmount * 20)),	
					std::vector<unsigned char>(tempEntryData.begin() + 59 + fLen + (sectionAmount * 20), tempEntryData.begin() + 63 + fLen + (sectionAmount * 20))
				};

				std::memcpy(&entryCompressedSize, segment.compressedSize.data(), segment.compressedSize.size());
				std::memcpy(&entryDecompressedSize, segment.decompressedSize.data(), segment.decompressedSize.size());
				std::memcpy(&startOffsetDec, segment.offset.data(), segment.offset.size());
				std::memcpy(&endOffsetDec, segment.alignedSize.data(), segment.alignedSize.size());
				decrypt(readFile, gameChoice, startOffsetDec, endOffsetDec, 1, entry.fileName);
				sectionAmount++;

				if (sectionSizeDec > 1) {
					float percentage = (float)(((i + 1) / (float)sectionSizeDec) * 100);
					std::cout << "Writing file entry " << entryIncrementor + 1 << "|" << entryNumberDec << ": " << entry.fileName << "\n";
				}
			}

			std::string absoluteP = filePath.substr(0, filePath.rfind("\\") + 1);
			std::replace(entry.fileName.begin(), entry.fileName.end(), '/', '\\');
			writeFileFunc(absoluteP, entry.fileName, bigFile);

			nextEntryOffset = nextEntryOffset + fLen + 43 + sectionAmount * 20;

			tempEntryData.clear();
			bigFile.clear();

			std::cout << "Written entry " << entryIncrementor + 1 << " | " << entryNumberDec << ": " << entry.fileName << "\n";
		}

		// end extraction
		std::cout <<
			"\n===========================================================================\n\n"
			"\n\n\nFILE EXTRACTED SUCCESFULLY!\a\n"
			"\nFile data path: " << filePath.substr(0, filePath.rfind("\\") + 1) << "media\\" <<
			"\n\nTotal file entries: " << entryNumberDec <<
			"\nTotal entries with sectioning enabled: " << sectionFileAmount <<
			"\n\nNPK version: NPK" << games[gameChoice].NPKver <<
			"\nFile IV: ";
			for (int i = 0; i < 16; i++) {
				std::cout << "0x" << std::hex << std::uppercase << (int)iv[i] << " ";
			}
			std::cout <<
			"\n\n";
	}

};


inline std::vector<unsigned char> decompressZSTD(std::vector<unsigned char> &entry) {
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

inline std::vector<unsigned char> decompressZLIB(std::vector<unsigned char> &entry) {

	std::vector<unsigned char> output;
	unsigned char buffer[4096];

	z_stream strm{};

	strm.next_in = entry.data();
	strm.avail_in = static_cast<unsigned int>(entry.size());

	int result = inflateInit2(&strm, -MAX_WBITS);

	do {
		strm.next_out = buffer;
		strm.avail_out = sizeof(buffer);

		result = inflate(&strm, Z_NO_FLUSH);

		size_t bufferData = sizeof(buffer) - strm.avail_out;

		output.insert(output.end(), buffer, buffer + bufferData);

	} while (result != Z_STREAM_END);

	inflateEnd(&strm);

	return output;
}
