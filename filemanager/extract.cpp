#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "plusaes.hpp"

class NPKmanip
{
private:
	const unsigned char iv[16];
public:
	
	
	void decrypt() {
		plusaes::decrypt_cbc();
	}

	void decompress() {

	}
};

int main()
{
	NPKmanip NPK;

	NPK.decrypt();
}
