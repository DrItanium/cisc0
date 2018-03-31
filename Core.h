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
				Count 
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
			class ImmediateFlag {
				public:
					ImmediateFlag() : _imm(false) { }
					bool isImmediate() const noexcept { return _imm; }
					void setImmediate(bool imm) noexcept { _imm = imm; }
				private:
					bool _imm;
			};
			template<bool value>
			class ConstantImmediateFlag {
				public:
					ConstantImmediateFlag() { };
					constexpr bool isImmediate() const noexcept { return value; }
			};

			class HasImmediateValue {
				public:
					HasImmediateValue() : _value(0) { }
					Address getImmediate() const noexcept { return _value; }
					void setImmediate(Address immediate) noexcept  { _value = immediate; }
				private:
					Address _value;
			};

			template<OperationCode op>
			struct Operation : HasDestination {
				Operation() { }
				~Operation() { }
				constexpr OperationCode getOpcode() const noexcept { return op; }
			};
			struct Set : Operation<OperationCode::Set>, HasBitmask {
				using Parent = Operation<OperationCode::Set>;
				using Parent::Parent;
				void setImmediate(Address value);
				Address getImmediate() const noexcept { return _immediate; }
				private:
					Address _immediate = 0;
			};
			struct Swap : Operation<OperationCode::Swap>, HasSource {
				using Parent = Operation<OperationCode::Swap>;
				using Parent::Parent;
			};
			struct Memory : Operation<OperationCode::Memory>, HasSource, HasBitmask {
				using Parent = Operation<OperationCode::Memory>;
				using Parent::Parent;
			};
			template<bool immediate>
			struct CompareGeneric  : Operation<OperationCode::Compare>, ConstantImmediateFlag<immediate> {
				enum class Style : byte { 
					Equals, 
					NotEquals, 
					LessThan, 
					GreaterThan, 
					LessThanOrEqualTo, 
					GreaterThanOrEqualTo, 
					MoveFromCondition, 
					MoveToCondition, 
				};
				using Parent = Operation<OperationCode::Compare>;
				using Parent::Parent;
				Style getStyle() const noexcept { return _style; }
				void setStyle(Style style) noexcept { _style = style; }
				private:
					Style _style;
			};
			struct CompareRegister : CompareGeneric<false>, HasSource {
				using Parent = CompareGeneric<false>;
				using Parent::Parent;
			};
			struct CompareImmediate : CompareGeneric<true>, HasBitmask, HasImmediateValue  {
				using Parent = CompareGeneric<true>;
				using Parent::Parent;
			};
			using Compare = std::variant<CompareRegister, CompareImmediate>;

		public:
			
	};
} // end namespace cisc0
#endif
