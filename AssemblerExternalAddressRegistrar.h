/*
 * syn
 * Copyright (c) 2013-2017, Joshua Scoggins and Contributors
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


#ifndef SYN_ASSEMBLER_EXTERNAL_ADDRESSES_H
#define SYN_ASSEMBLER_EXTERNAL_ADDRESSES_H
#include <string>
#include <cstdio>
#include <iostream>
#include <functional>
#include <map>
namespace syn {
	class AssemblerExternalAddressRegistrar {
		public:
            using Operation = std::function<void(void*)>;
			using AssemblerList = std::map<std::string, Operation>;
			AssemblerExternalAddressRegistrar();
			virtual ~AssemblerExternalAddressRegistrar();
            void install(void* env);
			void registerExternalAddress(const std::string& target, Operation op);
            // shim so that the register entry class can be used as is
            void addToRegistry(const std::string& target, Operation op) { registerExternalAddress(target, op); }
		private:
			AssemblerList assemblers;
	};

	extern AssemblerExternalAddressRegistrar externalAddressInstallerRegistry;
}

#endif // end SYN_ASSEMBLER_EXTERNAL_ADDRESSES_H
