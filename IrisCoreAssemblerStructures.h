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


#ifndef IRIS_CORE_ASSEMBLER_STRUCTURES_H__
#define IRIS_CORE_ASSEMBLER_STRUCTURES_H__
#include <sstream>
#include <typeinfo>
#include <iostream>
#include <map>
#include "Base.h"
#include "AssemblerBase.h"
#include "Problem.h"
#include "IrisCoreEncodingOperations.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/file_parser.hh>
#include <pegtl/contrib/raw_string.hh>
#include <pegtl/contrib/abnf.hh>
#include <pegtl/parse.hh>
#include <vector>
#include "IrisClipsExtensions.h"
#include "ClipsExtensions.h"
#include "IrisCoreAssemblerKeywords.h"

namespace iris {
#define DEF(type, str) \
    type stringTo ## type (const std::string& str) noexcept; \
    const std::string& str ## ToString ( type value ) noexcept
    DEF(ArithmeticOp, arithmeticOp);
    DEF(MoveOp, moveOp);
    DEF(JumpOp, jumpOp);
    DEF(CompareOp, compareOp);
    DEF(ConditionRegisterOp, conditionRegisterOp);
#undef DEF

    const std::string& translateRegister(byte index) noexcept;
    const std::string& translatePredicateRegister(byte index) noexcept;
    std::string translateInstruction(raw_instruction input) noexcept;

    enum class SectionType {
        Code,
        Data,
        Count,
    };
    struct AssemblerData {
        public:
            AssemblerData() noexcept;
            void setImmediate(word value) noexcept;
            bool shouldResolveLabel() const noexcept;
            dword encode() const noexcept;
        public:
            bool instruction;
            word address;
            word dataValue;

            byte group;
            byte operation;
            byte destination;
            byte source0;
            byte source1;
            bool hasLexeme;
            bool fullImmediate;
            std::string currentLexeme;
    };

    class AssemblerState : public syn::LabelTracker<word>, public syn::FinishedDataTracker<AssemblerData> {
        public:
            using LabelTracker = syn::LabelTracker<word>;
            AssemblerState() : _section(SectionType::Code) { }
            bool inCodeSection() const noexcept;
            bool inDataSection() const noexcept;
            void nowInCodeSection() noexcept;
            void nowInDataSection() noexcept;
            template<SectionType section>
            void changeSection() noexcept {
                static_assert(!syn::isErrorState<SectionType>(section), "Illegal state!");
                _section = section;
            }
            void setCurrentAddress(word value) noexcept;
            void registerLabel(const std::string& label) noexcept;
            word getCurrentAddress() const noexcept;
            void incrementCurrentAddress() noexcept;
        private:
            using AddressSpaceTracker = syn::AddressTracker<word>;
            AddressSpaceTracker data;
            AddressSpaceTracker code;
            SectionType _section;
    };
	enum class RegisterPositionType {
		DestinationGPR,
		Source0GPR,
		Source1GPR,
		PredicateDestination,
		PredicateInverseDestination,
		PredicateSource0,
		PredicateSource1,
		Count,
	};
	struct AssemblerInstruction : public AssemblerData {
		template<typename Input>
		AssemblerInstruction(const Input& in, AssemblerState& parent) noexcept {
			instruction = true;
			address = parent.getCurrentAddress();
		}

		template<typename Input>
		void success(const Input& in, AssemblerState& parent) {
			parent.incrementCurrentAddress();
			parent.addToFinishedData(*this);
		}
		void setField(RegisterPositionType type, byte value);
	};
	enum class AssemblerDirectiveAction {
		ChangeCurrentAddress,
		ChangeSection,
		DefineLabel,
		StoreWord,
		Count,
	};
	struct AssemblerDirective : public AssemblerData {
		template<typename I>
		AssemblerDirective(const I& in, AssemblerState& parent) {
			instruction = false;
			address = parent.getCurrentAddress();
			action = syn::defaultErrorState<decltype(action)>;
			section = syn::defaultErrorState<decltype(section)>;
		}
		template<typename Input>
		void success(const Input& in, AssemblerState& parent) {
			// TODO: insert code
			if (shouldChangeSectionToCode()) {
				parent.nowInCodeSection();
			} else if (shouldChangeSectionToData()) {
				parent.nowInDataSection();
			} else if (shouldChangeCurrentAddress()) {
				parent.setCurrentAddress(address);
			} else if (shouldDefineLabel()) {
				parent.registerLabel(currentLexeme);
			} else if (shouldStoreWord()) {
				if (parent.inDataSection()) {
					parent.addToFinishedData(*this);
					parent.incrementCurrentAddress();
				} else {
					throw syn::Problem("can't use a declare in a non data section!");
				}
			} else {
				throw syn::Problem("Undefined directive action!");
			}
		}
		bool shouldChangeSectionToCode() const noexcept;
		bool shouldChangeSectionToData() const noexcept;
		bool shouldChangeCurrentAddress() const noexcept;
		bool shouldDefineLabel() const noexcept;
		bool shouldStoreWord() const noexcept;

		AssemblerDirectiveAction action;
		SectionType section = syn::defaultErrorState<SectionType>;
	};
} // end namespace iris
#endif // end IRIS_CORE_ASSEMBLER_STRUCTURES_H__