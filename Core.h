/**
 * @file
 * concept of an execution core
 * @copyright
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


#ifndef _IRIS_CORE_H
#define _IRIS_CORE_H
#include <iostream>
#include <typeinfo>
#include <cstdint>
#include <variant>
#include <memory>
#include "Problem.h"

namespace cisc0 {
	using Address = uint32_t;
	using Integer = int32_t;
	using byte = uint8_t;
	using HalfAddress = uint16_t;
	using HalfInteger = int16_t;
	using DoubleAddress = uint64_t;
	using DoubleInteger = int64_t;
	using MemoryWord = HalfAddress;
	using RegisterIndex = byte;
	using Bitmask = byte;
	Address readRegisterValue(std::istream& in);
	constexpr bool extractImmediateBit(MemoryWord word) noexcept {
		return ((0b0000'0000'0001'0000 & word) >> 4) != 0;
	}
	constexpr MemoryWord make(byte lower, byte upper) noexcept {
		MemoryWord up = MemoryWord(upper) << 8;
		MemoryWord low = MemoryWord(lower);
		return up | low;
	}
	constexpr Address make(byte lowest, byte lower, byte higher, byte highest) noexcept {
		auto low = Address(make(lowest, lower));
		auto upper = Address(make(higher, highest)) << 16;
		return low | upper;
	}
	class Register {
		public:
			Register(Address value = 0, Address mask = 0xFFFFFFFF) : _address(value), _mask(mask) {
				maskContents();
			}
			Register(Integer value, Address mask = 0xFFFFFFFF) : _integer(value), _mask(mask) {
				maskContents();
			}
			Register(const Register& other) : _address(other._address), _mask(other._mask) { }
			Address getAddress() const noexcept { return _address; }
			Integer getInteger() const noexcept { return _integer; }
			bool getTruth() const noexcept { return _address != 0; }
			void setAddress(Address value) noexcept;
			void setInteger(Integer value) noexcept;
			void increment(Address incrementValue = 1) noexcept;
			void decrement(Address decrementValue = 1) noexcept;
			void setLowerHalf(MemoryWord value) noexcept;
			void setUpperHalf(MemoryWord value) noexcept;
			MemoryWord getUpperHalf() const noexcept { return MemoryWord((_address & 0xFFFF0000) >> 16); }
			MemoryWord getLowerHalf() const noexcept { return MemoryWord((_address & 0x0000FFFF)); }
			void setMask(Address mask) noexcept;
			Address getMask() const noexcept { return _mask; }
		private:
			void maskContents() noexcept;
		private:
			union {
				Address _address;
				Integer _integer;
			};
			Address _mask;
	};
	class Core {
		public:
			/**
			 * Constants that are always relevant to cisc0
			 */
			enum ArchitectureConstants  {
				RegisterCount = 16,
				MaxInstructionCount = 16,
				// we only have 16 registers to play with
				R15 = RegisterCount - 1,
				R14 = RegisterCount - 2,
				R13 = RegisterCount - 3,
				R12 = RegisterCount - 4,
				R11 = RegisterCount - 5,
				R10 = RegisterCount - 6,
				R9  = RegisterCount - 7,
				R8  = RegisterCount - 8,
				R7  = RegisterCount - 9,
				R6  = RegisterCount - 10,
				R5  = RegisterCount - 11,
				R4  = RegisterCount - 12,
				R3  = RegisterCount - 13,
				R2  = RegisterCount - 14,
				R1  = RegisterCount - 15,
				R0  = RegisterCount - 16,
				InstructionPointer = R15,
				StackPointer = R14,
				CallStackPointer = R13, // second stack
				/// used by load/store routines to describe the memory address
				AddressRegister = R12,
				/// used by load/store routines to describe the source or destination of the operation
				ValueRegister = R11,
                StringPointer = R10,
			};
			struct Extractable {
				public:
					Extractable() = default;
					~Extractable() = default;
					virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept = 0;

			};
			enum class OperationCode : byte { 
				Memory, 
				Arithmetic, 
				Shift, 
				Logical, 
				Compare, 
				Branch, 
				Move, 
				Set, 
				Swap, 
				Misc, 
			};
			struct HasBitmask {
				public:
					HasBitmask() : _mask(0) { }
					Bitmask getBitmask() const noexcept { return _mask; }
					void setBitmask(Bitmask mask) noexcept { _mask = mask & 0x0F; }
					void extractBitmask(MemoryWord word) noexcept { setBitmask(Bitmask((word & 0x0F00) >> 8)); }
					Address getExpandedBitmask() const noexcept {
						switch (_mask) {
							case 0b0000: 
								return 0x00000000;
							case 0b0001: 
								return 0x000000FF;
							case 0b0010:
								return 0x0000FF00;
							case 0b0011:
								return 0x0000FFFF;
							case 0b0100:
								return 0x00FF0000;
							case 0b0101:
								return 0x00FF00FF;
							case 0b0110:
								return 0x00FFFF00;
							case 0b0111:
								return 0x00FFFFFF;
							case 0b1000:
								return 0xFF000000;
							case 0b1001:
								return 0xFF0000FF;
							case 0b1010:
								return 0xFF00FF00;
							case 0b1011:
								return 0xFF00FFFF;
							case 0b1100:
								return 0xFFFF0000;
							case 0b1101:
								return 0xFFFF00FF;
							case 0b1110:
								return 0xFFFFFF00;
							case 0b1111:
								return 0xFFFFFFFF;
							default:
								throw Problem("Bad Index, terminating immediately!");
						}
					}
					MemoryWord getLowerMask() const noexcept {
						return MemoryWord(getExpandedBitmask() & 0x0000FFFF);
					}
					MemoryWord getUpperMask() const noexcept {
						return MemoryWord((getExpandedBitmask() & 0xFFFF0000) >> 16);
					}
				private:
					Bitmask _mask;
			};
			struct HasDestination {
				public:
					HasDestination() : _dest(0) { }
					RegisterIndex getDestination() const noexcept { return _dest; }
					void setDestination(RegisterIndex dest) noexcept { _dest = dest & 0x0F; }
					void extractDestination(MemoryWord word) noexcept { setDestination(RegisterIndex((word & 0xF000) >> 12)); }
				private:
					RegisterIndex _dest;
			};
			struct HasSource {
				public:
					HasSource() : _src(0) { }
					RegisterIndex getSource() const noexcept { return _src; }
					void setSource(RegisterIndex src) noexcept { _src = src & 0x0F; }
					void extractSource(MemoryWord word) noexcept { setSource(RegisterIndex((word & 0x0F00) >> 8)); }
				private:
					RegisterIndex _src;
			};

			struct HasImmediateValue {
				public:
					HasImmediateValue() : _value(0) { }
					Address getImmediate() const noexcept { return _value; }
					virtual void setImmediate(Address immediate) noexcept  { _value = immediate; }
					void extractImmediate(MemoryWord lower, MemoryWord upper) noexcept {
						auto up = Address(upper) << 16;
						auto bot = Address(lower);
						setImmediate(up | bot);
					}
				private:
					Address _value;
			};
			struct HasMaskableImmediateValue : public Extractable, public HasBitmask, public HasImmediateValue {
				public:
					using Parent0 = HasBitmask;
					using Parent1 = HasImmediateValue;
				public:
					HasMaskableImmediateValue() { };
					virtual void setImmediate(Address immediate) noexcept override {
						Parent1::setImmediate(immediate & getExpandedBitmask());
					}

					virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
						extractBitmask(a);
						extractImmediate(b, c);
					}

			};
			template<typename T>
			static constexpr T extractStyle(MemoryWord value, MemoryWord mask = 0b00000000'11100000, byte shift = 5) {
				return T((value & mask) >> shift);
			}
			template<typename S, MemoryWord mask, byte shift>
			struct HasStyle {
					static_assert(std::is_enum_v<S>, "HasStyle must be provided with an enum!");
				public:
					using Style = S;
					HasStyle() : _value(static_cast<S>(0)) { }
					Style getStyle() const noexcept { return _value; }
					void setStyle(Style s) noexcept { _value = s; }
					void extractStyle(MemoryWord value) noexcept { setStyle(static_cast<Style>((value & mask) >> shift)); }
				private:
					Style _value;
			};

			template<MemoryWord mask, byte shift>
			struct HasMemoryOffset {
				public:
					HasMemoryOffset() : _value(0) { }
					byte getMemoryOffset() const noexcept { return _value; }
					void setMemoryOffset(byte v) noexcept { _value = v; }
					void extractMemoryOffset(MemoryWord value) noexcept {
						setMemoryOffset(byte((value & mask) >> shift));
					}
				private:
					byte _value;
			};



			enum class CompareStyle : byte { 
				Equals, 
				NotEquals, 
				LessThan, 
				GreaterThan, 
				LessThanOrEqualTo, 
				GreaterThanOrEqualTo, 
				MoveFromCondition, 
				MoveToCondition, 
			};
			struct CompareGeneric  : public Extractable, public HasDestination, public HasStyle<CompareStyle, 0b0000000011100000, 5> {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					extractDestination(a);
					extractStyle(a);
				}
			};
			struct CompareRegister : CompareGeneric, HasSource {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					CompareGeneric::extract(a, b, c);
					extractSource(a);
				}
			};
			struct CompareImmediate : CompareGeneric, HasMaskableImmediateValue { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					CompareGeneric::extract(a, b, c);
					HasMaskableImmediateValue::extract(a, b, c);
				}
			};
			struct CompareMoveFromCondition : public Extractable, public HasDestination {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					extractDestination(a);
				}
			};
			struct CompareMoveToCondition : public Extractable, public HasDestination {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					extractDestination(a);
				}
			};

			using Compare = std::variant<CompareRegister, CompareImmediate, CompareMoveFromCondition, CompareMoveToCondition>;

			enum class ArithmeticStyle : byte { 
				Add,
				Sub,
				Mul,
				Div,
				Rem,
				Min,
				Max,
			};
			struct ArithmeticGeneric : Extractable, HasDestination, HasStyle<ArithmeticStyle, 0b0000000011100000, 5> { 
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					extractStyle(a);
					extractDestination(a);
				}
			};
			struct ArithmeticRegister : ArithmeticGeneric, HasSource { 
				using Parent0 = ArithmeticGeneric;

				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					Parent0::extract(a, b, c);
					extractSource(a);
				}
			};
			struct ArithmeticImmediate : ArithmeticGeneric, HasMaskableImmediateValue {
				using Parent0 = ArithmeticGeneric;
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					Parent0::extract(a, b, c);
					HasMaskableImmediateValue::extract(a, b, c);
				}
			};
			using Arithmetic = std::variant<ArithmeticRegister, ArithmeticImmediate>;
			enum class LogicalStyle : byte { 
				And, 
				Or, 
				Xor, 
				Nand, 
				Not 
			};
			struct LogicalGeneric : Extractable, HasDestination, HasStyle<LogicalStyle, 0b0000000011100000, 5> { 
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					extractDestination(a);
					extractStyle(a);
				}
			};
			struct LogicalRegister : LogicalGeneric, HasSource { 
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					LogicalGeneric::extract(a, b, c);
					extractSource(a);
				}
			};
			struct LogicalImmediate : LogicalGeneric, HasMaskableImmediateValue { 
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					LogicalGeneric::extract(a, b, c);
					HasMaskableImmediateValue::extract(a, b, c);
				}
			};
			using Logical = std::variant<LogicalRegister, LogicalImmediate>;
			struct ShiftGeneric : Extractable, HasDestination {
				bool shiftLeft() const noexcept { return _shiftLeft; }
				void setShiftLeft(bool value) noexcept { _shiftLeft = value; }
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					setShiftLeft(((a & 0b0000000000100000) >> 5) != 0);
					extractDestination(a);
				}
				private:
					bool _shiftLeft = false;
			};
			struct ShiftRegister : public ShiftGeneric, public HasSource {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					ShiftGeneric::extract(a, b, c);
					extractSource(a);
				}
			};
			struct ShiftImmediate : ShiftGeneric {
				void setShiftAmount(byte value) noexcept { _imm5 = value & 0b00011111; }
				byte getShiftAmount() const noexcept { return _imm5; }
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					ShiftGeneric::extract(a, b, c);
					setShiftAmount(byte((a & 0b0000111110000000) >> 7));
				}
				private:
					byte _imm5;
			};
			using Shift = std::variant<ShiftRegister, ShiftImmediate>;
			struct BranchGeneric : Extractable {
				public:
					bool conditionallyEvaluate() const noexcept { return _conditionallyEvaluate; }
					bool performCall() const noexcept { return _performCall; }
					void setConditionallyEvaluate(bool value) noexcept { _conditionallyEvaluate = value; }
					void setPerformCall(bool value) noexcept { _performCall = value; }
					virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
						setPerformCall(((a & 0b0000000000100000) >> 5) != 0);
						setConditionallyEvaluate(((a & 0b0000000001000000) >> 6) != 0);
					}
				private:
					bool _conditionallyEvaluate = false;
					bool _performCall = false;
			};
			struct BranchRegister : BranchGeneric, HasDestination {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					BranchGeneric::extract(a, b, c);
					extractDestination(a);
				}
			};
			struct BranchImmediate : BranchGeneric, HasImmediateValue {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					BranchGeneric::extract(a, b, c);
					extractImmediate(b, c);
				}
			};
			using Branch = std::variant<BranchRegister, BranchImmediate>;
			enum class MemoryStyle : byte {
				Load,
				Store,
				Push,
				Pop,
			};
			struct MemoryGeneric : Extractable, HasBitmask {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractBitmask(a);
				}
			};
			// use the destination field to store an offset
			struct MemoryLoad : MemoryGeneric, HasMemoryOffset<0xF000, 12> {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					MemoryGeneric::extract(a, b, c);
					extractMemoryOffset(a);
				}
			};
			struct MemoryStore : MemoryGeneric, HasMemoryOffset<0xF000, 12> { 
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					MemoryGeneric::extract(a, b, c);
					extractMemoryOffset(a);
				}
			};
			struct MemoryPush : MemoryGeneric, HasDestination { 
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					MemoryGeneric::extract(a, b, c);
					extractDestination(a);
				}
			};
			struct MemoryPop : MemoryGeneric, HasDestination {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					MemoryGeneric::extract(a, b, c);
					extractDestination(a);
				}
			};
			using Memory = std::variant<MemoryLoad, MemoryStore, MemoryPush, MemoryPop>;

			struct Move : Extractable, HasDestination, HasSource, HasBitmask {
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					extractDestination(a);
					extractSource(a);
					extractBitmask(a);
				}
			};
			struct Set : HasDestination, HasMaskableImmediateValue { 
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					extractDestination(a);
					HasMaskableImmediateValue::extract(a, b, c);
				}
			};
			struct Swap : public Extractable, public HasDestination, public HasSource { 
				virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
					extractDestination(a);
					extractSource(a);
				}

			};
			enum class MiscStyle {
				Return,
				Terminate,
                PutCharacter,
                GetCharacter,
                ReadWord,
			};
			struct Return : Extractable { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override { }
			};
			struct Terminate : Extractable { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override { }
			};
            struct PutCharacter : Extractable, HasDestination {
                virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
                    extractDestination(a);
                }
            };
            struct GetCharacter : Extractable, HasDestination {
                virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
                    extractDestination(a);
                }
            };
            /**
             * Extract a single word (forth style) from input.
             * The memory location used to store the word is found in the StringPointer register
             * The number of characters read is placed in memory at the start of where string pointer is
             * pointing to. The destination denotes the maximum size of the string. Use -1 for "unlimited"
             *
             * This operation can be implemented in terms of GetCharacter but after doing it enough times 
             * I realize that this operation should be a feature of the core not something
             * to stumble against. 
             */
            struct ReadWord : Extractable, HasDestination {
                virtual void extract(MemoryWord a, MemoryWord b = 0, MemoryWord c = 0) noexcept override {
                    extractDestination(a);
                }
            };
			using Misc = std::variant<Return, Terminate, PutCharacter, GetCharacter>;

			using Operation = std::variant<Compare, Arithmetic, Logical, Shift, Branch, Memory, Move, Set, Swap, Misc>;
		public:
			static constexpr Address defaultMemoryCapacity = 0xFFFFFF + 1;
			Core(Address memoryCapacity = defaultMemoryCapacity);
			void storeWord(Address addr, MemoryWord value);
			Address popSubroutineAddress() noexcept;
			MemoryWord popSubroutineWord() noexcept;
			MemoryWord popParameterWord() noexcept;
			Address popParameterAddress() noexcept;
			void pushParameterWord(MemoryWord value) noexcept;
			void pushParameterAddress(Address value) noexcept;
			void pushSubroutineWord(MemoryWord value) noexcept;
			void pushSubroutineAddress(Address value) noexcept;
			void run();
			void install(std::istream& in);
			void dump(std::ostream& out);
			Register& getRegister(RegisterIndex index);
		private:
			MemoryWord loadWord(Address addr);
			template<byte index>
			Register& getRegister() noexcept {
				return _registers[index & 0x0F];
			}
			Register& getDestination(const HasDestination&);
			Register& getSource(const HasSource&);
			Register& getPC();
			Register& getValueRegister() noexcept;
			Register& getAddressRegister() noexcept;
			template<typename T>
			void variantInvoke(const T& value) {
				std::visit([this](auto&& x) { invoke(x); }, value);
			}
			MemoryWord nextWord();
			void invoke(const Return& value);
			void invoke(const Terminate& value);
            void invoke(const PutCharacter& value);
            void invoke(const GetCharacter& value);
            void invoke(const ReadWord& value);
			void invoke(const Misc& value);
			void invoke(const Swap& value);
			void invoke(const Set& value);
			void invoke(const Move& value);
			void invoke(const Memory& value);
			void invoke(const MemoryPop& value);
			void invoke(const MemoryPush& value);
			void invoke(const MemoryStore& value);
			void invoke(const MemoryLoad& value);
			void invoke(const Branch& value);
			void invoke(const BranchRegister& value);
			void invoke(const BranchImmediate& value);
			void invoke(const Shift& value);
			void invoke(const ShiftRegister& value);
			void invoke(const ShiftImmediate& value);
			void invoke(const Logical& value);
			void invoke(const LogicalRegister& value);
			void invoke(const LogicalImmediate& value);
			void invoke(const Arithmetic& value);
			void invoke(const ArithmeticRegister& value);
			void invoke(const ArithmeticImmediate& value);
			void invoke(const Compare& value);
			void invoke(const CompareRegister& value);
			void invoke(const CompareImmediate& value);
			void invoke(const CompareMoveToCondition& value);
			void invoke(const CompareMoveFromCondition& value);
			void invoke(const Operation& value);
			Operation decode();
			void decode(MemoryWord first, Return& value);
			void decode(MemoryWord first, Terminate& value);
			void decode(MemoryWord first, GetCharacter& value);
			void decode(MemoryWord first, PutCharacter& value);
            void decode(MemoryWord first, ReadWord& value);
			void decode(MemoryWord first, Misc& value);
			void decode(MemoryWord first, Swap& value);
			void decode(MemoryWord first, Set& value);
			void decode(MemoryWord first, Move& value);
			void decode(MemoryWord first, Memory& value);
			void decode(MemoryWord first, MemoryPop& value);
			void decode(MemoryWord first, MemoryPush& value);
			void decode(MemoryWord first, MemoryStore& value);
			void decode(MemoryWord first, MemoryLoad& value);
			void decode(MemoryWord first, Branch& value);
			void decode(MemoryWord first, BranchRegister& value);
			void decode(MemoryWord first, BranchImmediate& value);
			void decode(MemoryWord first, Shift& value);
			void decode(MemoryWord first, ShiftRegister& value);
			void decode(MemoryWord first, ShiftImmediate& value);
			void decode(MemoryWord first, Logical& value);
			void decode(MemoryWord first, LogicalRegister& value);
			void decode(MemoryWord first, LogicalImmediate& value);
			void decode(MemoryWord first, Arithmetic& value);
			void decode(MemoryWord first, ArithmeticRegister& value);
			void decode(MemoryWord first, ArithmeticImmediate& value);
			void decode(MemoryWord first, Compare& value);
			void decode(MemoryWord first, CompareRegister& value);
			void decode(MemoryWord first, CompareImmediate& value);
			void decode(MemoryWord first, CompareMoveToCondition& value);
			void decode(MemoryWord first, CompareMoveFromCondition& value);
			template<typename T, typename ImmediateType, typename RegisterType>
			void decodeOnImmediateBit(MemoryWord first, T& value) {
				if (extractImmediateBit(first)) {
					value = ImmediateType();
				} else {
					value = RegisterType();
				}
				std::visit([this, first](auto&& value) { decode(first, value); }, value);
			}
			template<typename T>
			void decodeImmediateValue(MemoryWord first, T& value) {
				auto second = MemoryWord(0);
				auto third = MemoryWord(0);
				value.extractBitmask(first);
				if (auto lowerMask = value.getLowerMask(); lowerMask != 0) {
					second = nextWord();
				}
				if (auto upperMask = value.getUpperMask(); upperMask != 0) {
					third = nextWord();
				}
				value.extract(first, second, third);
			}
		private:
			Address _capacity;
			std::unique_ptr<Register[]> _registers;
			std::unique_ptr<MemoryWord[]> _memory;
			bool _conditionRegister = false;
			bool _keepExecuting = true;
	};
} // end namespace cisc0
#endif
