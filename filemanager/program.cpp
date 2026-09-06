#include "unpack.h"
#include "repack.h"


int main() {

	while (stopProgram == false) {
		std::cout << "Please select the game you are working with: \n";
		for (unsigned int i = 0; i < sizeof(games) / sizeof(NPKgame); i++) {
			std::cout << "\n   " << i << ":    " << games[i].name << "\n";
		}
		std::cout << "\n";

		bool next = false;
		while (next == false) {
			gameChoice = _getch();
			gameChoice = gameChoice - '0';

			if (gameChoice < sizeof(games) / sizeof(NPKgame)) {
				std::cout << "\nSelected game: " << games[gameChoice].name;
				next = true;
			}
			else std::cout << "\nNot a valid game choice!\n";
		}



		bool next2 = false;
		while (next2 == false) {

			std::cout << "\n\nPlease input the path of your NPK file: ";
			std::getline(std::cin, filePath);
			filePath.erase(std::remove(filePath.begin(), filePath.end(), '"'), filePath.end());

			if (!std::filesystem::exists(filePath)) {
				std::cout << "\nYour file path is not valid/doesn't exist!\n";
			}
			else if (std::filesystem::path(filePath).extension().string() != ".npk") {
				std::cout << "\nWritten/dropped file is not an NPK!";
			}
			else if (std::filesystem::exists(filePath) && std::filesystem::path(filePath).extension().string() == ".npk") {
				if (std::filesystem::file_size(filePath) > 300000000) {
					std::cout <<
						"\nSelected NPK is " << (int)(std::filesystem::file_size(filePath) / 1000000) <<
						"MB! Extracting files over 300MB take some storage and might take a bit longer, are you sure? \nY|N\n";
					if (question() == true) next2 = true;
				}
				else next2 = true;
			}
		}


		NPKunpack NPK(filePath);

		auto start = std::chrono::high_resolution_clock::now();

		NPK.readHeader();
		NPK.decrypt(NPK.readFile, gameChoice, headerSize, NPK.dataOffsetDec, 0, "NULL");
		std::cout << "\nEntry array decrypted... Starting file entries decryption.\n\n";
		NPK.getEntries(NPK.entryBuffer);

		auto end = std::chrono::high_resolution_clock::now();
		float duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		std::cout <<
			"\n\n\n===========================================================================\n" <<
			"\nAction execution time was " << (float)((float)duration / 1000) << " seconds.\n\n\n\n";

		std::cout << "Would you like to interact with another NPK? Y|N\n";
		if (question() == false) {
			stopProgram = true;
			std::cout << "\n\n";
		}
	}

	return 0;
}