/*
 * cisc0 
 * Copyright (c) 2013-2018, Joshua Scoggins and Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Core.h"
#include "Problem.h"
#include <iostream>
#include <fstream>
#include <list>
#include <type_traits>


void usage(const std::string& name) {
	std::cerr << name << ": <path-to-object> [more objects, -o fileName]" << std::endl;
}
using Address = cisc0::Address;
int main(int argc, char** argv) {
	Address capacity = cisc0::Core::defaultMemoryCapacity;
	if (argc == 1) {
		usage(argv[0]);
		return 1;
	}
	bool findOutput = false;
	std::string output = "iris.img";
	std::list<std::string> files;
	for (int i = 1; i < argc; ++i) {
		std::string value = argv[i];
		if (findOutput) {
			output = value;
			findOutput = false;
		} else if (value == "-o") {
			findOutput = true;
		} else {
			files.emplace_back(value);
		}
	}
	struct InstallationTarget {
		InstallationTarget(cisc0::Address address, cisc0::MemoryWord value, bool viewAsRegister = false) : _address(address), _value(value), _viewAsRegister(viewAsRegister) { }
		cisc0::Address _address;
		cisc0::MemoryWord _value;
		bool _viewAsRegister = false;
	};
	std::list<InstallationTarget> installs;
	// TODO: continue this and generate a system image based off of it
	for (auto const & path : files) {
		std::ifstream in(path.c_str(), std::ios::binary);
		auto getByte = [&in]() {
			union {
				char c;
				cisc0::byte b;
			} tmp;
			tmp.c = in.get();
			return tmp.b;
		};
		auto getWord = [getByte]() {
			auto lower = cisc0::MemoryWord(getByte());
			auto upper = cisc0::MemoryWord(getByte()) << 8;
			return lower | upper;
		};
		auto getAddress = [getWord]() {
			auto lower = cisc0::Address(getWord());
			auto upper = cisc0::Address(getWord()) << 16;
			return lower | upper;
		};
		while (in) {
			// read each entry to see what to do with it
			auto section = getWord();
            if (in.gcount() == 0) {
                break;
            }
			auto address = getAddress();
			auto value = getWord();
			switch (section) {
				case 0: // capacity
					capacity = capacity > address ? capacity : address ;
					break;
				case 1: // installation of register
					installs.emplace_back(address, value, true);
					break;
				case 2: // installation of memory
					installs.emplace_back(address, value);
					break;
				default:
					std::cerr << "Got an illegal section, terminating..." << std::endl;
					std::cerr << "\tThe culprint file is: " << path << std::endl;
					return 1;
			}
		}
		if (in.bad()) {
			std::cerr << "A really bad error occurred while installing section entries, terminating..." << std::endl;
			std::cerr << "\tThe culprit file is: " << path << std::endl;
			return 1;
		}
		in.close();
	}
	cisc0::Core core(capacity);
	for (const auto & a : installs) {
		if (a._viewAsRegister) {
			// look at it backwards!
			core.getRegister(cisc0::byte(0x0F & a._value)).setAddress(a._address);
		} else {
			core.storeWord(a._address, a._value);
		}
	}
	if (!output.empty()) {
		std::ofstream file(output.c_str(), std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "could not open: " << output << " for writing!" << std::endl;
			return 1;
		} else {
			core.dump(file);
		}
		file.close();
	}
	return 0;
}
