#pragma once

#ifndef INFO
#define INFO
#include "information.h"
#endif

class NPKrepack { // all the file repacking functions. WARNING: extremely unoptimised and very badly written... suggestions are greatly appreciated
private:

	std::vector<unsigned long long> entriesDataOffsetVector;
	const unsigned int allocationSize = 16777216;
	unsigned long long entriesOffSetTotal = 32;
	std::vector<unsigned int> entrySegmentAmount;
	unsigned int segmentCounter = 0;
	unsigned int sectionWritingStream = 0;

	std::ofstream writeFile;
	std::ifstream readFile;

	std::vector<NPKentry> entries;
	std::vector<NPKentry::segmentData> segments;
	unsigned int chunk = ((games[gameChoice].NPKver - 1) * 65536);
	std::string newFileName;

	inline void writeToFile(std::vector<unsigned char>& data, std::string fileName) {
		for (unsigned long long i = 0; i < data.size(); i++) {
			writeFile << data[i];
		}
	}

public:
	std::string file;
	NPKrepack(const std::string filename) {
		file = filename;
		newFileName = file.substr(0, file.rfind('\\')) + "\\" +  file.substr(file.rfind('\\') + 1) +  "~.npk";
		writeFile.open(newFileName, std::ios::binary);
	}


	unsigned char iv[16] = {};


	unsigned int fileCounter = 0;
	unsigned int folderCounter = 0;

public:

	inline void allocateEntryBytes() {
		std::vector<unsigned char> allocation(allocationSize);
		writeToFile(allocation, newFileName);
	}

	inline void randomiseIV(int size) {
		for (unsigned int i = 0; i < size; i++) {
			iv[i] = rand() % 256;
		}
	}

	inline void startRepack(auto& filepath) {
		for (const auto& entry : std::filesystem::recursive_directory_iterator(filepath)) {
			if (entry.is_regular_file()) {
				assembleEntryData(entry, fileCounter);
				fileCounter++;
			}
			else {
				folderCounter++;
			}
		}
	}

	inline std::vector<unsigned char> encrypt(std::vector<unsigned char> data, std::vector<unsigned char> key) {
		const unsigned long encryptedSize = plusaes::get_padded_encrypted_size(data.size());
		//std::cout << " " << encryptedSize;
		std::vector<unsigned char> encryptedData(encryptedSize);

		plusaes::encrypt_cbc((unsigned char*)data.data(), data.size(), &key[0], key.size(), &iv, &encryptedData[0], encryptedSize, true);
		data.resize(encryptedSize);

		return encryptedData;
	}


	inline void assembleEntryData(const auto& filepath, unsigned int entriesID) {

		entries.emplace_back();

		readFile.open(filepath, std::ios::binary);
		std::string ext = std::filesystem::path(filepath).extension().string();

		std::vector<unsigned char> fileData;
		while (readFile >> std::noskipws >> byte) {
			fileData.push_back(byte);
		}

		std::vector<unsigned char> writingFileData;

		std::string entriesName = filepath.path().string().substr(filepath.path().string().rfind("media\\"));
		std::replace(entriesName.begin(), entriesName.end(), '\\', '/');
		unsigned short entriesNameLen = entriesName.length();
		unsigned int fileSize = std::filesystem::file_size(filepath);
		unsigned int sectionSize = std::ceil((float)((float)fileSize / ((float)chunk)));
		
		std::cout << "\nPacking entry : " << entriesName << "\n";
		if (ext == ".nut") {
			fileData = NUTpatcher(fileData);
		}

		if (ext != ".png" && ext != ".ogg" && ext != ".mpg" && ext != ".jpg") {
			entries[entriesID].enableSegmentation = 0;
		}
		else {
			entries[entriesID].enableSegmentation = 1;
		}

		entries[entriesID].fileNameLength.insert(entries[entriesID].fileNameLength.end(),
			reinterpret_cast<unsigned char*>(&entriesNameLen),
			reinterpret_cast<unsigned char*>(&entriesNameLen) + sizeof(entriesNameLen)
		);

		entries[entriesID].fileName = entriesName;

		entries[entriesID].realSize.insert(entries[entriesID].realSize.end(),
			reinterpret_cast<unsigned char*>(&fileSize),
			reinterpret_cast<unsigned char*>(&fileSize) + sizeof(fileSize)
		);

		std::vector<unsigned char> hash(picosha2::k_digest_size);
		picosha2::hash256(fileData, hash.begin(), hash.end());
		entries[entriesID].SHA256.insert(entries[entriesID].SHA256.end(), hash.begin(), hash.end());

		entries[entriesID].sectionSize.insert(entries[entriesID].sectionSize.end(),
			reinterpret_cast<unsigned char*>(&sectionSize),
			reinterpret_cast<unsigned char*>(&sectionSize) + sizeof(sectionSize)
		);

		for (unsigned i = 0; i < sectionSize; i++) {
			segments.emplace_back();
			
			std::vector<unsigned char> segmentData;

			unsigned int dSize = fileSize - (chunk * i);
			if (dSize > chunk) {
				dSize = chunk;
			}
			segmentData.insert(segmentData.end(), fileData.begin() + chunk * i, fileData.begin() + chunk * i + dSize);

			segments[segmentCounter].decompressedSize.insert(segments[segmentCounter].decompressedSize.end(),
				reinterpret_cast<unsigned char*>(&dSize),
				reinterpret_cast<unsigned char*>(&dSize) + sizeof(dSize)
			);

			if (ext != ".png" && ext != ".ogg" && ext != ".mpg" && ext != ".jpg") {
				switch (games[gameChoice].NPKver) {
				case 2:
					writingFileData = compressZLIB(segmentData);
					break;
				case 3:
					writingFileData = compressZSTD(segmentData);
					break;
				}
			}

			unsigned int compressedFileSize = writingFileData.size();
			segments[segmentCounter].compressedSize.insert(segments[segmentCounter].compressedSize.end(),
				reinterpret_cast<unsigned char*>(&compressedFileSize),
				reinterpret_cast<unsigned char*>(&compressedFileSize) + sizeof(compressedFileSize)
			);

			unsigned int alignedFileSize = plusaes::get_padded_encrypted_size(compressedFileSize);
			segments[segmentCounter].alignedSize.insert(segments[segmentCounter].alignedSize.end(),
				reinterpret_cast<unsigned char*>(&alignedFileSize),
				reinterpret_cast<unsigned char*>(&alignedFileSize) + sizeof(alignedFileSize)
			);

			writingFileData = encrypt(writingFileData, games[gameChoice].key);
			writeToFile(writingFileData, newFileName);


			entriesDataOffsetVector.emplace_back(writingFileData.size());

			entriesOffSetTotal = entriesOffSetTotal + 20;
			segmentData.clear();
			segmentCounter++;
		}

		readFile.close();
		fileData.clear();
		writingFileData.clear();
		entriesOffSetTotal = entriesOffSetTotal + entriesNameLen + 43;
		entrySegmentAmount.emplace_back(sectionSize);
	}

	inline void finishWrite(int entryAmount, int folderAmount) {
		writeEntries(entryAmount, segmentCounter);


		std::cout <<
			"\n\n\n===========================================================================\n\n" << 
			"\n\n\nFILE PACKED SUCCESFULLY!\a\n" <<
			"\nFile written at: " << newFileName <<
			"\n\nTotal file entries: " << entryAmount <<
			"\n\nNPK version: NPK" << games[gameChoice].NPKver <<
			"\nFile IV: ";
		for (int i = 0; i < 16; i++) {
			std::cout << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)iv[i] << " ";
		}
		std::cout <<
			"\n\n";
		std::cout << std::dec;
	}

private:
	inline void writeEntries(int entryAmount, int segmentAmount) {
		writeFile.seekp(0);

		entriesOffSetTotal = plusaes::get_padded_encrypted_size(entriesOffSetTotal - 32);
		unsigned int dataOffset = entriesOffSetTotal;
		std::vector<unsigned char> fileHeader; // prepare HEADER for the file.

		std::cout << "\n\nWriting file header...";

		std::string NPK2string = "NPK2";
		std::string NPK3string = "NPK3";
		switch (games[gameChoice].NPKver) {
		case 2:
			fileHeader.insert(fileHeader.end(), NPK2string.begin(), NPK2string.end());
			fileHeader.push_back(2);
			break;
		case 3:
			fileHeader.insert(fileHeader.end(), NPK3string.begin(), NPK3string.end());
			fileHeader.push_back(1);
			break;
		}
		fileHeader.insert(fileHeader.end(), 3, 0);
		for (unsigned int i = 0; i < ivSize; i++) {
			fileHeader.push_back(iv[i]);
		}
		fileHeader.insert(fileHeader.end(),
			reinterpret_cast<unsigned char*>(&entryAmount),
			reinterpret_cast<unsigned char*>(&entryAmount) + sizeof(entryAmount)
		);
		fileHeader.insert(fileHeader.end(),
			reinterpret_cast<unsigned char*>(&dataOffset),
			reinterpret_cast<unsigned char*>(&dataOffset) + sizeof(dataOffset)
		);

		std::cout << newFileName;
		writeToFile(fileHeader, newFileName);


		unsigned int sectionStream = 0;
		std::vector<unsigned char> entryList;
		entriesOffSetTotal = allocationSize;
		for (unsigned int i = 0; i < segmentAmount; i++) {
			segments[i].offset.insert(segments[i].offset.end(),
				reinterpret_cast<unsigned char*>(&entriesOffSetTotal),
				reinterpret_cast<unsigned char*>(&entriesOffSetTotal) + sizeof(entriesOffSetTotal)
			);
			entriesOffSetTotal = entriesOffSetTotal + entriesDataOffsetVector[i];
		}
		for (unsigned int i = 0; i < entryAmount; i++) {
			unsigned int incrementor = i;
			entryList.push_back(entries[i].enableSegmentation);
			entryList.insert(entryList.end(), entries[i].fileNameLength.begin(), entries[i].fileNameLength.end());
			entryList.insert(entryList.end(), entries[i].fileName.begin(), entries[i].fileName.end());
			entryList.insert(entryList.end(), entries[i].realSize.begin(), entries[i].realSize.end());
			entryList.insert(entryList.end(), entries[i].SHA256.begin(), entries[i].SHA256.end());
			entryList.insert(entryList.end(), entries[i].sectionSize.begin(), entries[i].sectionSize.end());
			for (unsigned int i = 0; i < entrySegmentAmount[incrementor]; i++) {
				unsigned int tempSecSize;
				std::memcpy(&tempSecSize, entries[incrementor].sectionSize.data(), entries[incrementor].sectionSize.size());
				if (i > 0) {
					sectionStream++;
				}
				entryList.insert(entryList.end(), segments[sectionStream + incrementor].offset.begin(), segments[sectionStream + incrementor].offset.end());
				entryList.insert(entryList.end(), segments[sectionStream + incrementor].alignedSize.begin(), segments[sectionStream + incrementor].alignedSize.end());
				entryList.insert(entryList.end(), segments[sectionStream + incrementor].compressedSize.begin(), segments[sectionStream + incrementor].compressedSize.end());
				entryList.insert(entryList.end(), segments[sectionStream + incrementor].decompressedSize.begin(), segments[sectionStream + incrementor].decompressedSize.end());
			}
		}
		entryList = encrypt(entryList, games[gameChoice].key);

		std::cout << "\n\nWriting entry table...";
		writeToFile(entryList, newFileName);
		writeFile.close();
		entryList.clear();
		fileHeader.clear();
		std::cout << std::dec;
	}

};

inline std::vector<unsigned char> compressZSTD(std::vector<unsigned char> &entry) {
	size_t maxSize = ZSTD_compressBound(entry.size());
	std::vector<unsigned char> compressedData(maxSize);

	size_t result = ZSTD_compress(
		compressedData.data(),
		compressedData.size(),
		entry.data(),
		entry.size(),
		3
	);

	if (ZSTD_isError(result)) {
		std::cout << ZSTD_getErrorName(result) << '\n';
	} else {
		compressedData.resize(result);
	}

	return compressedData;
}

inline std::vector<unsigned char> compressZLIB(std::vector<unsigned char>& entry) {
	z_stream strm{};

	int ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);

	unsigned long bound = deflateBound(&strm, entry.size());
	std::vector<unsigned char> output(bound);

	if (ret != Z_OK) {
		std::cout << "\n\nCompression failed with error code " << ret << "\n\n";
		return output;
	}

	strm.next_in = entry.data();
	strm.avail_in = entry.size();

	strm.next_out = output.data();
	strm.avail_out = output.size();

	deflate(&strm, Z_FINISH);
	output.resize(strm.total_out);

	deflateEnd(&strm);

	return output;
}
