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
	union Word {
		Word() : address(0) { }
		Word(Address a) : address(a) { }
		Word(Integer i) : integer(i) { }
		Word(const Word& other) : Word(other.address) { }
		Address address;
		Integer integer;
	};
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
				Return, 
				Complex, 
			};
			class HasBitmask {
				public:
					HasBitmask() : _mask(0) { }
					Bitmask getBitmask() const noexcept { return _mask; }
					void setBitmask(Bitmask mask) noexcept { _mask = mask & 0x0F; }
				private:
					Bitmask _mask;
			};
			class HasDestination {
				public:
					HasDestination() : _dest(0) { }
					RegisterIndex getDestination() const noexcept { return _dest; }
					void setDestination(RegisterIndex dest) noexcept { _dest = dest & 0x0F; }
				private:
					RegisterIndex _dest;
			};
			class HasSource {
				public:
					HasSource() : _src(0) { }
					RegisterIndex getSource() const noexcept { return _src; }
					void setSource(RegisterIndex src) noexcept { _src = src & 0x0F; }
				private:
					RegisterIndex _src;
			};

			class HasImmediateValue {
				public:
					HasImmediateValue() : _value(0) { }
					Address getImmediate() const noexcept { return _value; }
					void setImmediate(Address immediate) noexcept  { _value = immediate; }
				private:
					Address _value;
			};
			template<typename S>
			class HasStyle {
					static_assert(std::is_enum_v<S>, "HasStyle must be provided with an enum!");
				public:
					using Style = S;
					HasStyle() : _value(static_cast<S>(0)) { }
					Style getStyle() const noexcept { return _value; }
					void setStyle(Style s) noexcept { _value = s; }
				private:
					Style _value;
			};

			struct Set : HasDestination, HasBitmask, HasImmediateValue { };
			struct Swap : HasDestination, HasSource { };
			struct Memory : HasDestination, HasSource, HasBitmask { };

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
			struct CompareGeneric  : HasDestination, HasStyle<CompareStyle> { };
			struct CompareRegister : CompareGeneric, HasSource { };
			struct CompareImmediate : CompareGeneric, HasBitmask, HasImmediateValue  { };
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
			struct ArithmeticGeneric : HasDestination, HasStyle<ArithmeticStyle> { };
			struct ArithmeticRegister : ArithmeticGeneric, HasSource { };
			struct ArithmeticImmediate : ArithmeticGeneric, HasBitmask, HasImmediateValue { };
			using Arithmetic = std::variant<ArithmeticRegister, ArithmeticImmediate>;
			enum class LogicalStyle : byte { 
				And, 
				Or, 
				Xor, 
				Nand, 
				Not 
			};
			struct LogicalGeneric : HasDestination, HasStyle<LogicalStyle> { };
			struct LogicalRegister : LogicalGeneric, HasSource { };
			struct LogicalImmediate : LogicalGeneric, HasBitmask, HasImmediateValue { };
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
		public:
			
	};
} // end namespace cisc0
#endif
