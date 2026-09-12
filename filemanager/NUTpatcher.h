#pragma once

#ifndef INFO
#define INFO
#include "information.h"
#endif

inline std::vector<unsigned char> NUTpatcher(std::vector<unsigned char> &NUTfile) {

	const unsigned char strPrefix[4] = {0x10, 0x00, 0x00, 0x08};
	const unsigned char diaPrefix[2] = {0x0D, 0x0A};
	const unsigned char diaSuffix[3] = {0x0D, 0x0A, 0x10};
	const unsigned char trap[4] = {'T', 'R', 'A', 'P'};
	std::vector<unsigned char> endHeader = {'L', 'I', 'A', 'T'};

	if (NUTfile.size() == 0) {
		std::cout << "\nSomething went wrong in NUT file patching.\n";
		return NUTfile;
	}

	std::vector<unsigned char> NUTdata = NUTfile;
	auto findEnd = std::find_end(NUTdata.begin(), NUTdata.end(), endHeader.begin(), endHeader.end());

	uint32_t endFileHeader = std::distance(NUTdata.begin(), findEnd) + 4;
	uint32_t startFileHeader = endFileHeader - 16;

	for (uint32_t i = 0; i < NUTdata.size(); i++) {

		/*if (NUTdata[i] == 0x20) {
			NUTdata[i] = 0x00;
		}*/ // unreliable for now
		if (NUTdata[i] == strPrefix[0] &&
			NUTdata[i + 1] == strPrefix[1] &&
			NUTdata[i + 2] == strPrefix[2] &&
			NUTdata[i + 3] == strPrefix[3] &&
			NUTdata[i + 8] == diaPrefix[0] &&
			NUTdata[i + 9] == diaPrefix[1]
			) {

			for (uint32_t in = 0; in < 4294967295; in++) {
				uint32_t diaLen = in;

				if (NUTdata[i + in + 8] == diaSuffix[0] && NUTdata[i + in + 9] == diaSuffix[1] && NUTdata[i + in + 10] == diaSuffix[2]) {
					diaLen = diaLen + 2;
					std::memcpy(NUTdata.data() + i + 4, &diaLen, sizeof(diaLen));
					i = i + 7 + diaLen;	
					break;
					
				}
				else if (NUTdata[i + in + 8] == trap[0] && NUTdata[i + in + 9] == trap[1] && NUTdata[i + in + 10] == trap[2] && NUTdata[i + in + 11] == trap[3]) {
					std::memcpy(NUTdata.data() + i + 4, &diaLen, sizeof(diaLen));
					i = i + diaLen;
					break;
				}
			}
		}
	}

	std::memcpy(NUTdata.data() + 8, &startFileHeader, sizeof(startFileHeader));
	std::memcpy(NUTdata.data() + 12, &endFileHeader, sizeof(endFileHeader));

	std::cout << "NUT file patched.";

	return NUTdata;

}