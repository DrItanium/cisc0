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
			class HasBitmask {
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
			class HasDestination {
				public:
					HasDestination() : _dest(0) { }
					RegisterIndex getDestination() const noexcept { return _dest; }
					void setDestination(RegisterIndex dest) noexcept { _dest = dest & 0x0F; }
					void extractDestination(MemoryWord word) noexcept { setDestination(RegisterIndex((word & 0xF000) >> 12)); }
				private:
					RegisterIndex _dest;
			};
			class HasSource {
				public:
					HasSource() : _src(0) { }
					RegisterIndex getSource() const noexcept { return _src; }
					void setSource(RegisterIndex src) noexcept { _src = src & 0x0F; }
					void extractSource(MemoryWord word) noexcept { setSource(RegisterIndex((word & 0x0F00) >> 8)); }
				private:
					RegisterIndex _src;
			};

			class HasImmediateValue {
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
			class HasMaskableImmediateValue : public HasBitmask, HasImmediateValue {
				public:
					using Parent0 = HasBitmask;
					using Parent1 = HasImmediateValue;
				public:
					HasMaskableImmediateValue() { };
					virtual void setImmediate(Address immediate) noexcept override {
						Parent1::setImmediate(immediate & getExpandedBitmask());
					}
			};
			template<typename S, MemoryWord mask, byte shift>
			class HasStyle {
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
			class HasMemoryOffset {
				public:
					HasMemoryOffset() : _value(0) { }
					byte getMemoryOffset() const noexcept { return _value; }
					void setMemoryOffset(byte v) noexcept { _value = v; }
					void extract(MemoryWord value) noexcept {
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
			struct CompareGeneric  : HasDestination, HasStyle<CompareStyle, 0b0000000011100000, 5> { };
			struct CompareRegister : CompareGeneric, HasSource { };
			struct CompareImmediate : CompareGeneric, HasMaskableImmediateValue { };
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
			struct ArithmeticGeneric : HasDestination, HasStyle<ArithmeticStyle, 0b0000000011100000, 5> { };
			struct ArithmeticRegister : ArithmeticGeneric, HasSource { };
			struct ArithmeticImmediate : ArithmeticGeneric, HasMaskableImmediateValue { };
			using Arithmetic = std::variant<ArithmeticRegister, ArithmeticImmediate>;
			enum class LogicalStyle : byte { 
				And, 
				Or, 
				Xor, 
				Nand, 
				Not 
			};
			struct LogicalGeneric : HasDestination, HasStyle<LogicalStyle, 0b0000000011100000, 5> { };
			struct LogicalRegister : LogicalGeneric, HasSource { };
			struct LogicalImmediate : LogicalGeneric, HasMaskableImmediateValue { };
			using Logical = std::variant<LogicalRegister, LogicalImmediate>;
			struct ShiftGeneric : HasDestination {
				bool shiftLeft() const noexcept { return _shiftLeft; }
				void setShiftLeft(bool value) noexcept { _shiftLeft = value; }
				private:
					bool _shiftLeft = false;
			};
			struct ShiftRegister : ShiftGeneric, HasSource { };
			struct ShiftImmediate : ShiftGeneric {
				void setShiftAmount(byte value) noexcept { _imm5 = value & 0b00011111; }
				byte getShiftAmount() const noexcept { return _imm5; }
				private:
					byte _imm5;
			};
			using Shift = std::variant<ShiftRegister, ShiftImmediate>;
			struct BranchGeneric {
				public:
					bool performLink() const noexcept { return _performLink; }
					bool performCall() const noexcept { return _performCall; }
					void setPerformLink(bool value) noexcept { _performLink = value; }
					void setPerformCall(bool value) noexcept { _performCall = value; }
				private:
					bool _performLink = false;
					bool _performCall = false;
			};
			struct BranchRegister : BranchGeneric, HasDestination { };
			struct BranchImmediate : BranchGeneric, HasImmediateValue { };
			using Branch = std::variant<BranchRegister, BranchImmediate>;
			enum class MemoryStyle : byte {
				Load,
				Store,
				Push,
				Pop,
			};
			struct MemoryGeneric : HasBitmask {
				bool indirectBitSet() const noexcept { return _indirect; }
				void setIndirectBit(bool v) noexcept { _indirect = v; }
				private:
					bool _indirect = false;
			};
			// use the destination field to store an offset
			struct MemoryLoad : MemoryGeneric, HasMemoryOffset<0xF000, 12> { };
			struct MemoryStore : MemoryGeneric, HasMemoryOffset<0xF000, 12> { };
			struct MemoryPush : MemoryGeneric, HasDestination { };
			struct MemoryPop : MemoryGeneric, HasDestination { };
			using Memory = std::variant<MemoryLoad, MemoryStore, MemoryPush, MemoryPop>;

			struct Move : HasDestination, HasSource, HasBitmask { };
			struct Set : HasDestination, HasBitmask, HasImmediateValue { };
			struct Swap : HasDestination, HasSource { };
			enum class MiscStyle {
				Return,
				Terminate,
			};
			struct Return { };
			struct Terminate : HasDestination { };
			using Misc = std::variant<Return, Terminate>;
		public:
			
	};
} // end namespace cisc0
#endif
