//UNUSED FILE. DO NOT COMPILE.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

#include "plusaes.hpp"

int main() {

	std::string file;
	std::cout << "Please insert the path of your file: ";
	std::getline(std::cin, file);
	

	std::ifstream fileDecrypted(file, std::ios::binary);
	std::ofstream fileEncrypted("encrypt.npk~", std::ios::binary);
	std::vector<unsigned char> fileArr;
	char byte;
	while (fileDecrypted.get(byte))
	{
		fileArr.push_back(static_cast<unsigned char>(byte));
	}

	const unsigned long data_size = plusaes::get_padded_encrypted_size(fileArr.size());
	const std::vector<unsigned char> key = {
		0xE7, 0xE8, 0xA5, 0xF9, 0x9B, 0xAF, 0x7C, 0x73,
		0xAE, 0x6B, 0xDF, 0x3D, 0x8C, 0x90, 0x26, 0x2F,
		0xF2, 0x50, 0x25, 0xA1, 0x2D, 0xB5, 0x39, 0xF9,
		0xCF, 0xD6, 0xE8, 0xE5, 0x79, 0x75, 0xB7, 0x98 };
	const unsigned char iv[16] = {
		0x62, 0x64, 0xC1, 0xFC, 0x5B, 0xC8, 0x86, 0x09,
		0x60, 0xBB, 0xD1, 0x6D, 0x58, 0x5A, 0xAA, 0xC4
	}; //script.npk

	/*const unsigned char iv[16] = {
		0x4D, 0x8C, 0xE1, 0xED, 0x13, 0xAE, 0x58, 0x80, 
		0xBF, 0x9E, 0x05, 0x18, 0x27, 0x8E, 0x57, 0x74
	}; // cg.npk*/
	//unsigned long padded_size = fileArr.size();

	std::cout << fileArr.size() << " " << data_size << "\n";
	std::vector<unsigned char> EncryptedFileArr(fileArr.size());
	//decryptedFileArr.push_back(5000000);

	plusaes::decrypt_cbc(&fileArr[0], fileArr.size(), &key[0], 32, &iv, &EncryptedFileArr[0], fileArr.size(), NULL);

	//plusaes::encrypt_cbc(&fileArr[0], fileArr.size(), &key[0], 32, &iv, &EncryptedFileArr[0], fileArr.size(), false);

	std::cout << EncryptedFileArr.size();
	std::cout << "\n File decrypted successfully. Writing to new file.";

	for (int i = 0; i < EncryptedFileArr.size(); i++) {
		fileEncrypted << EncryptedFileArr[i];
	}

	std::cout << "\n File created and written to succesfully.\a";

	// 62 64 C1 FC 5B C8 86 09 60 BB D1 6D 58 5A AA C4
	

	return 0;
}