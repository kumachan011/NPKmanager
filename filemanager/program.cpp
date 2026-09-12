#include "unpack.h"
#include "repack.h"

int main() {

	srand(time(0));

	while (stopProgram == false) {

		char interact;

		std::cout << "Please select the game you are working with: \n";
		for (unsigned int i = 0; i < sizeof(games) / sizeof(NPKgame); i++) {
			std::cout << "\n	" << i << ":	" << games[i].name << "\n";
		}
		std::cout << "\n";

		bool next = false;
		while (next == false) {
			gameChoice = _getch();
			gameChoice = gameChoice - '0';

			if (gameChoice < sizeof(games) / sizeof(NPKgame)) {
				std::cout << "\nSelected game: " << games[gameChoice].name;
				std::cout << "\n\nWhat do you want to do with the selected game?\n\n	0:	Unpack NPK\n\n	1:	Repack NPK\n";
				bool choose = false;
				while (choose == false) {
					interact = _getch();
					interact = interact - '0';
					if (interact > 1) {
						std::cout << "\nI don't understand what you mean with " << (char)(interact + '0') << "\n";
					} else if (interact <= 1) {
						next = true;
						choose = true;
					}
				}
			}
			else std::cout << "\nNot a valid game choice!\n";
		}


		if (interact == 0) {
			bool startUnpack = false;
			while (startUnpack == false) {

				std::cout << "\n\nPlease input the path of your NPK file: ";
				std::getline(std::cin, filePath);
				filePath.erase(std::remove(filePath.begin(), filePath.end(), '"'), filePath.end());

				if (!std::filesystem::exists(filePath)) {
					std::cout << "\nYour file path is not valid/doesn't exist!\n";
				} else if (std::filesystem::path(filePath).extension().string() != ".npk") {
					std::cout << "\nWritten/dropped file is not an NPK!";
				} else if (std::filesystem::exists(filePath) && std::filesystem::path(filePath).extension().string() == ".npk") {
					if (std::filesystem::file_size(filePath) > 300000000) {
						std::cout <<
							"\nSelected NPK is " << (int)(std::filesystem::file_size(filePath) / 1000000) <<
							"MB!  Files over 300MB take a significant amount of storage and might take a bit longer, are you sure? \nY|N\n";
						if (question() == true) startUnpack = true;
					} else startUnpack = true;
				}
			}


			std::cout << "\n\nStarting... \n\n";
			auto start = std::chrono::high_resolution_clock::now(); // start unpacking

			NPKunpack NPK(filePath);
			NPK.readHeader();
			NPK.decrypt(NPK.readFile, gameChoice, headerSize, NPK.dataOffsetDec, 0, "NULL");
			std::cout << "\nEntry array decrypted... Starting file entries decryption.\n\n";
			NPK.getEntries(NPK.entryBuffer);

			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

			std::cout <<
				"\n\n\n===========================================================================\n" <<
				"\nAction execution time was " << (float)((float)duration / 1000) << " seconds.\n\n\n\n";
		}
		if (interact == 1) {
			bool startRepack = false;
			while (startRepack == false) {

				std::cout << "\n\nPlease input the path of your media file. Make sure it is the item folder (eg. dx, script, cg), not the media folder! ";
				std::getline(std::cin, filePath);
				filePath.erase(std::remove(filePath.begin(), filePath.end(), '"'), filePath.end());

				if (!std::filesystem::exists(filePath)) {
					std::cout << "\nYour file path is not valid/doesn't exist!\n";
				} else if (std::filesystem::is_directory(filePath)) {
					if (filePath.substr(filePath.find_last_of("\\") + 1) == "media") {
						std::cout << "\nI told you to not choose the media folder. Choose a folder inside it!\n";
					} else if (filePath.find("media") == std::string::npos) {
						std::cout << "\nThe current folder is not part of a media folder. You wouldn't want to pack something you didn't want now, right?\n";
					} else if (filePath.substr(filePath.find_last_of("\\") + 1) != "media" && filePath.find("media") != std::string::npos) startRepack = true;
				}
			}

			std::cout << "\n\nStarting... \n\n";
			auto start = std::chrono::high_resolution_clock::now(); // start repacking

			std::filesystem::remove(filePath.substr(0, filePath.rfind('\\')) + filePath.substr(filePath.rfind('\\') + 1) + "~.npk");
			NPKrepack NPK(filePath);
			NPK.allocateEntryBytes();
			NPK.randomiseIV(ivSize);
			NPK.startRepack(NPK.file);
			NPK.finishWrite(NPK.fileCounter, NPK.folderCounter);

			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

			std::cout <<
				"\n\n\n===========================================================================\n" <<
				"\nAction execution time was " << (float)((float)duration / 1000) << " seconds.\n\n\n\n";
		}

		std::cout << "Another interaction? Y|N\n";
		if (question() == false) {
			stopProgram = true;
			std::cout << "\n\n";
		}
	}

	return 0;
}