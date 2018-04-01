/**
 * @file
 * implementations of Core routines
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


#include "Core.h"
#include "Problem.h"

namespace cisc0 {
	void Register::increment(Address incrementValue) noexcept {
		_address += incrementValue;
		maskContents();
	}
	void Register::decrement(Address decrementValue) noexcept {
		_address -= decrementValue;
		maskContents();
	}
	void Register::setMask(Address mask) noexcept {
		_mask = mask;
		maskContents();
	}
	void Register::maskContents() noexcept {
		_address = _address & _mask;
	}
	void Register::setAddress(Address a) noexcept {
		_address = a;
		maskContents();
	}
	void Register::setInteger(Integer a) noexcept {
		_integer = a;
		maskContents();
	}

	Register& Core::getRegister(RegisterIndex idx) {
		return _registers[idx & 0x0F];
	}
	Register& Core::getPC() {
		return getRegister<Core::ArchitectureConstants::InstructionPointer>();
	}
	Register& Core::getDestination(const Core::HasDestination& dest) {
		return getRegister(dest.getDestination());
	}
	Register& Core::getSource(const Core::HasSource& src) {
		return getRegister(src.getSource());
	}
	Core::Core(Address memCap) : _capacity(memCap) {
		_memory = std::make_unique<MemoryWord[]>(memCap);
		_registers = std::make_unique<Register[]>(16);
		for ( int i = 0; i < 16; ++i) {
			_registers[i] = 0;
		}
		for (Address a = 0; a < memCap; ++a) {
			_memory[a] = 0;
		}
		auto capacityMask = _capacity - 1;
		_registers[Core::ArchitectureConstants::InstructionPointer].setMask(capacityMask);
		_registers[Core::ArchitectureConstants::AddressRegister].setMask(capacityMask);
		_registers[Core::ArchitectureConstants::StackPointer].setMask(capacityMask);
		_registers[Core::ArchitectureConstants::CallStackPointer].setMask(capacityMask);
	}
	MemoryWord Core::loadWord(Address addr) {
		if (addr >= _capacity) {
			throw Problem("Illegal address!");
		} else {
			return _memory[addr];
		}
	}
	void Core::storeWord(Address addr, MemoryWord value) {
		if (addr >= _capacity) {
			throw Problem("Illegal address!");
		} else {
			_memory[addr] = value;
		}
	}
	MemoryWord Core::nextWord() {
		auto& pc = getPC();
		MemoryWord curr = loadWord(pc.getAddress());
		pc.increment(_capacity - 1);
		return curr;
	}
	MemoryWord Core::popSubroutineWord() noexcept {
		auto& subroutine = getRegister<Core::ArchitectureConstants::CallStackPointer>();
		auto value = loadWord(subroutine.getAddress());
		subroutine.increment();
		return value;
	}
	Address Core::popSubroutineAddress() noexcept {
		auto lowerHalf = Address(popSubroutineWord());
		auto upperHalf = Address(popSubroutineWord()) << 16;
		return lowerHalf | upperHalf;
	}
	MemoryWord Core::popParameterWord() noexcept {
		auto& sp = getRegister<Core::ArchitectureConstants::StackPointer>();
		auto value = loadWord(sp.getAddress());
		sp.increment();
		return value;
	}
	Address Core::popParameterAddress() noexcept {
		auto lower = Address(popParameterWord());
		auto upper = Address(popParameterWord()) << 16;
		return lower | upper;
	}
	void Core::invoke(const Core::Return&) {
		auto newAddr = popSubroutineAddress();
		getPC().setAddress(newAddr);
	}

	void Core::invoke(const Core::Terminate&) {
		_keepExecuting = false;
	}

	void Core::invoke(const Core::Misc& value) {
		variantInvoke(value);
	}

	void Core::invoke(const Core::Swap& value) {
		auto& a = getDestination(value);
		auto& b = getSource(value);
		auto c = a.getAddress();
		a.setAddress(b.getAddress());
		b.setAddress(c);
	}

	void Core::invoke(const Core::Set& value) {
		getDestination(value).setAddress(value.getImmediate());
	}

	void Core::invoke(const Core::Move& value) {
		getDestination(value).setAddress(getSource(value).getAddress());
	}

	void Core::invoke(const Core::Memory& value) {
		variantInvoke(value);
	}

	void Core::invoke(const Core::MemoryPop& value) {

	}

	void Core::invoke(const Core::MemoryPush& value) {

	}

	void Core::invoke(const Core::MemoryStore& value) {
		auto addr = getRegister<Core::ArchitectureConstants::AddressRegister>().getAddress() + value.getMemoryOffset();
		auto& val = getRegister<Core::ArchitectureConstants::ValueRegister>();
		auto lowerMask = value.getLowerMask();
		auto upperMask = value.getUpperMask();
		if (lowerMask == 0 && upperMask == 0) {
		} else if (lowerMask == 0xFFFF && upperMask == 0) {
			storeWord(addr, MemoryWord((val.getAddress() & 0x0000FFFF)));
		} else if (lowerMask == 0x0000 && upperMask == 0xFFFF) {
			storeWord(addr + 1, MemoryWord((val.getAddress() & 0xFFFF0000) >> 16));
		} else if (lowerMask == 0xFFFF && upperMask == 0xFFFF) {
			storeWord(addr, MemoryWord(val.getAddress() & 0x0000FFFF));
			storeWord(addr + 1, MemoryWord((val.getAddress() & 0xFFFF0000) >> 16));
		} else {
			if (lowerMask != 0) {
				auto value = loadWord(addr) & ~lowerMask;
				auto newValue = val.getLowerHalf() & lowerMask;
				storeWord(addr, value | newValue);
			} 

			if (upperMask != 0) {
				auto value = loadWord(addr + 1) & ~upperMask;
				auto newValue = val.getUpperHalf() & upperMask;
				storeWord(addr + 1, value | newValue);
			}
		}
	}

	void Core::invoke(const Core::MemoryLoad& value) {
		auto addr = getRegister<Core::ArchitectureConstants::AddressRegister>().getAddress() + value.getMemoryOffset();
		auto& val = getRegister<Core::ArchitectureConstants::ValueRegister>();
		bool readLower = false;
		bool readUpper = false;
		switch (value.getBitmask()) {
			case 0b0000:
				break;
			case 0b0001:
			case 0b0010:
			case 0b0011:
				// lower half only!
				readLower = true;
				break;
			case 0b0100:
			case 0b1000:
			case 0b1100:
				readUpper = true;
				break;
			default:
				readLower = true;
				readUpper = true;
				break;
		}
		auto lower = readLower ? Address(loadWord(addr)) : 0;
		auto upper = readUpper ? Address(loadWord(addr + 1)) << 16 : 0;
		val.setInteger((lower | upper) & value.getExpandedBitmask());
	}

	void Core::invoke(const Core::Branch& value) {
		variantInvoke(value);
	}

	void Core::invoke(const Core::BranchRegister& value) {

	}

	void Core::invoke(const Core::BranchImmediate& value) {

	}

	void Core::invoke(const Core::Shift& value) {
		variantInvoke(value);
	}
	constexpr Address performShift(bool shiftLeft, Address base, Address shift) noexcept {
		if (shiftLeft) {
			return base << shift;
		} else {
			return base >> shift;
		}
	}
	void Core::invoke(const Core::ShiftRegister& value) {
		auto& dest = getDestination(value);
		auto& src = getSource(value);
		dest.setAddress(performShift(value.shiftLeft(), dest.getAddress(), src.getAddress()));
	}

	void Core::invoke(const Core::ShiftImmediate& value) {
		auto& dest = getDestination(value);
		dest.setAddress(performShift(value.shiftLeft(), dest.getAddress(), value.getShiftAmount()));
	}

	void Core::invoke(const Core::Logical& value) {
		variantInvoke(value);
	}

	void Core::invoke(const Core::LogicalRegister& value) {
		auto& dest = getDestination(value);
		auto& src = getSource(value);
		switch (value.getStyle()) {
			case LogicalStyle::And:
				dest.setAddress(dest.getAddress() & src.getAddress());
				break;
			case LogicalStyle::Or:
				dest.setAddress(dest.getAddress() | src.getAddress());
				break;
			case LogicalStyle::Xor:
				dest.setAddress(dest.getAddress() ^ src.getAddress());
				break;
			case LogicalStyle::Not:
				dest.setAddress(~src.getAddress());
				break;
			case LogicalStyle::Nand:
				dest.setAddress(~(dest.getAddress() & src.getAddress()));
				break;
			default:
				throw Problem("Illegal logical style!");
		}
	}

	void Core::invoke(const Core::LogicalImmediate& value) {
		auto& dest = getDestination(value);
		switch (value.getStyle()) {
			case LogicalStyle::And:
				dest.setAddress(dest.getAddress() & value.getImmediate());
				break;
			case LogicalStyle::Or:
				dest.setAddress(dest.getAddress() | value.getImmediate());
				break;
			case LogicalStyle::Xor:
				dest.setAddress(dest.getAddress() ^ value.getImmediate());
				break;
			case LogicalStyle::Not:
				dest.setAddress(~value.getImmediate());
				break;
			case LogicalStyle::Nand:
				dest.setAddress(~(dest.getAddress() & value.getImmediate()));
				break;
			default:
				throw Problem("Illegal logical style!");
		}

	}

	void Core::invoke(const Core::Arithmetic& value) {
		variantInvoke(value);
	}

	void Core::invoke(const Core::ArithmeticRegister& value) {

	}

	void Core::invoke(const Core::ArithmeticImmediate& value) {

	}

	void Core::invoke(const Core::Compare& value) {
		variantInvoke(value);
	}

	void Core::invoke(const Core::CompareRegister& value) {

	}

	void Core::invoke(const Core::CompareImmediate& value) {

	}

	void Core::invoke(const Core::Operation& value) {
		variantInvoke(value);
	}


} // end namespace cisc0

