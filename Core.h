/**
 * @file
 * concept of an execution core
 * @copyright
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


#ifndef _IRIS_CORE_H
#define _IRIS_CORE_H
#include <iostream>
#include <typeinfo>
#include <cstdint>
#include <variant>

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
	class Core {
		public:
			struct Extractable {
				public:
					Extractable() = default;
					~Extractable() = default;
					virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c ) noexcept = 0;
					inline void extract(MemoryWord a, MemoryWord b) noexcept {
						extract(a, b, 0);
					}
					inline void extract(MemoryWord a) noexcept {
						extract(a, 0);
					}

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
								throw "Bad Index!";
						}
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
			struct HasMaskableImmediateValue : Extractable, HasBitmask, HasImmediateValue {
				public:
					using Parent0 = HasBitmask;
					using Parent1 = HasImmediateValue;
				public:
					HasMaskableImmediateValue() { };
					virtual void setImmediate(Address immediate) noexcept override {
						Parent1::setImmediate(immediate & getExpandedBitmask());
					}

					virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
						extractBitmask(a);
						extractImmediate(b, c);
					}

			};
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
			struct CompareGeneric  : Extractable, HasDestination, HasStyle<CompareStyle, 0b0000000011100000, 5> {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractDestination(a);
					extractStyle(a);
				}
			};
			struct CompareRegister : CompareGeneric, HasSource {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
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
			using Compare = std::variant<CompareRegister, CompareImmediate>;

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
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractStyle(a);
					extractDestination(a);
				}
			};
			struct ArithmeticRegister : ArithmeticGeneric, HasSource { 
				using Parent0 = ArithmeticGeneric;

				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
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
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractDestination(a);
					extractStyle(a);
				}
			};
			struct LogicalRegister : LogicalGeneric, HasSource { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					LogicalGeneric::extract(a, b, c);
					extractSource(a);
				}
			};
			struct LogicalImmediate : LogicalGeneric, HasMaskableImmediateValue { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
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
			struct ShiftRegister : ShiftGeneric, HasSource {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					ShiftGeneric::extract(a, b, c);
					extractSource(a);
				}
			};
			struct ShiftImmediate : ShiftGeneric {
				void setShiftAmount(byte value) noexcept { _imm5 = value & 0b00011111; }
				byte getShiftAmount() const noexcept { return _imm5; }
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					ShiftGeneric::extract(a, b, c);
					setShiftAmount(byte((a & 0b0000111110000000) >> 7));
				}
				private:
					byte _imm5;
			};
			using Shift = std::variant<ShiftRegister, ShiftImmediate>;
			struct BranchGeneric : Extractable {
				public:
					bool performLink() const noexcept { return _performLink; }
					bool performCall() const noexcept { return _performCall; }
					void setPerformLink(bool value) noexcept { _performLink = value; }
					void setPerformCall(bool value) noexcept { _performCall = value; }
					virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
						setPerformCall(((a & 0b0000000000100000) >> 5) != 0);
						setPerformLink(((a & 0b0000000001000000) >> 6) != 0);
					}
				private:
					bool _performLink = false;
					bool _performCall = false;
			};
			struct BranchRegister : BranchGeneric, HasDestination {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
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
				bool indirectBitSet() const noexcept { return _indirect; }
				void setIndirectBit(bool v) noexcept { _indirect = v; }
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractBitmask(a);
					setIndirectBit(((a & 0b0000000001000000) >> 6) != 0);
				}
				private:
					bool _indirect = false;
			};
			// use the destination field to store an offset
			struct MemoryLoad : MemoryGeneric, HasMemoryOffset<0xF000, 12> {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					MemoryGeneric::extract(a, b, c);
					extractMemoryOffset(a);
				}
			};
			struct MemoryStore : MemoryGeneric, HasMemoryOffset<0xF000, 12> { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					MemoryGeneric::extract(a, b, c);
					extractMemoryOffset(a);
				}
			};
			struct MemoryPush : MemoryGeneric, HasDestination { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					MemoryGeneric::extract(a, b, c);
					extractDestination(a);
				}
			};
			struct MemoryPop : MemoryGeneric, HasDestination {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					MemoryGeneric::extract(a, b, c);
					extractDestination(a);
				}
			};
			using Memory = std::variant<MemoryLoad, MemoryStore, MemoryPush, MemoryPop>;

			struct Move : Extractable, HasDestination, HasSource, HasBitmask {
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractDestination(a);
					extractSource(a);
					extractBitmask(a);
				}
			};
			struct Set : HasDestination, HasMaskableImmediateValue { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractDestination(a);
					HasMaskableImmediateValue::extract(a, b, c);
				}
			};
			struct Swap : Extractable, HasDestination, HasSource { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractDestination(a);
					extractSource(a);
				}

			};
			enum class MiscStyle {
				Return,
				Terminate,
			};
			struct Return : Extractable { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override { }
			};
			struct Terminate : Extractable, HasDestination { 
				virtual void extract(MemoryWord a, MemoryWord b, MemoryWord c) noexcept override {
					extractDestination(a);
				}
			};
			using Misc = std::variant<Return, Terminate>;

			using Operation = std::variant<Compare, Arithmetic, Logical, Shift, Branch, Memory, Move, Set, Swap, Misc>;
		public:
			static constexpr Address defaultMemoryCapacity = 0xFFFFFF + 1;
			Core(Address memoryCapacity = defaultMemoryCapacity);
			void storeMemory(Address addr, MemoryWord value);
		private:
			template<typename T>
			void variantInvoke(const T& value) {
				std::visit([this](auto&& x) { invoke(x); }, value);
			}
			MemoryWord nextWord();
			void invoke(const Return& value);
			void invoke(const Terminate& value);
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
			void invoke(const Operation& value);
			Operation decode();
			void decode(MemoryWord first, Return& value);
			void decode(MemoryWord first, Terminate& value);
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
	};
} // end namespace cisc0
#endif
