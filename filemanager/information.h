#pragma once

#ifndef HEADERS
#define HEADERS
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <chrono>
#include <conio.h>
#include <algorithm>

#include "plusaes.hpp" // uses the header only plusaes AES decryption/encryption
#include "zstd.h" // facebook/zsdt
#include "zlib.h" //uses the official library from zlib.net for the INFLATE and DEFLATE algorithm
#endif

static bool stopProgram = false;

static unsigned char byte;
static const short headerSize = 32;

static unsigned long int fileSize;
static std::string filePath;
static unsigned int gameChoice;

inline std::vector<unsigned char> decompressZSTD(std::vector<unsigned char>&);
inline std::vector<unsigned char> decompressZLIB(std::vector<unsigned char>&);
inline bool isDeflate(std::vector<unsigned char>&);


struct NPKgame {
	const char* name;
	unsigned short NPKver;
	const std::vector<unsigned char> key;
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

		std::vector<unsigned char> decompressedSize; // entry after decompression
	};
};

static NPKgame games[] = {
	{"You and Me and Her: a Love Story", 3, {
		0xE7, 0xE8, 0xA5, 0xF9, 0x9B, 0xAF, 0x7C, 0x73,
		0xAE, 0x6B, 0xDF, 0x3D, 0x8C, 0x90, 0x26, 0x2F,
		0xF2, 0x50, 0x25, 0xA1, 0x2D, 0xB5, 0x39, 0xF9,
		0xCF, 0xD6, 0xE8, 0xE5, 0x79, 0x75, 0xB7, 0x98 }
	},
	{"Tokyo Necro", 2, {
		0x92, 0x0A, 0x2C, 0xBD, 0x4A, 0xF0, 0x19, 0xC9,
		0x5F, 0x4E, 0x94, 0x2D, 0x05, 0xF9, 0x06, 0xC7,
		0xA6, 0x81, 0x26, 0xCD, 0x85, 0x84, 0x6E, 0x5A,
		0x66, 0x92, 0xC7, 0xCA, 0x04, 0x83, 0xD1, 0x85 }
	},
	//{"test", 3, {}}
// more to be added when game key is requested/found
};

inline void progressBar(unsigned short barWidth, float progress) {
	std::cout << "|";
	for (unsigned int i = 0; i <= barWidth; i++) {
		if (i < barWidth * (progress / 99)) {

			std::cout << "=";
		}
		else {
			std::cout << "-";
		}
	}
	std::cout << "| (" << progress << "%)\n\n";
}

inline bool question() {
	char answer;
	while (true) {
		answer = _getch();
		if (toupper(answer) == 'Y') return true;
		else if (toupper(answer) == 'N') return false;
		else std::cout << "\nI don't understand what you mean with " << (char)(toupper(answer)) << "\n";
	}
}