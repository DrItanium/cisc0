#include "iris16.h"
#include "sim_registration.h"
#include <functional>
#include <sstream>


namespace iris16 {
	Core* newCore() noexcept {
		return new iris16::Core();
	}


	Core::~Core() { }

	void Core::setInstructionMemory(word address, dword value) noexcept {
		instruction[address] = value;
	}

	void Core::setDataMemory(word address, word value) noexcept {
		data[address] = value;
	}

	void Core::installprogram(std::istream& stream) {
		char wordBuf[sizeof(word)] = { 0 };
		char dwordBuf[sizeof(dword)] = { 0 };
		for (auto i = 0; i < ArchitectureConstants::RegisterCount; ++i) {
			stream.read(wordBuf, sizeof(word));
			gpr[i] = iris16::encodeWord(wordBuf[0], wordBuf[1]);
		}
		for (auto i = 0; i < ArchitectureConstants::AddressMax; ++i) {
			stream.read(wordBuf, sizeof(word));
			setDataMemory(i, iris16::encodeWord(wordBuf[0], wordBuf[1]));
		}
		for (auto i = 0; i < ArchitectureConstants::AddressMax; ++i) {
			stream.read(dwordBuf, sizeof(dword));
			setInstructionMemory(i, iris16::encodeDword(dwordBuf[0], dwordBuf[1], dwordBuf[2], dwordBuf[3]));
		}
		for (auto i = 0; i < ArchitectureConstants::AddressMax; ++i) {
			stream.read(wordBuf, sizeof(word));
			stack[i] = iris16::encodeWord(wordBuf[0], wordBuf[1]);
		}
	}

	void Core::dump(std::ostream& stream) {
		char wordBuf[sizeof(word)] = { 0 };
		char dwordBuf[sizeof(dword)] = { 0 };
		for (auto i = 0; i < ArchitectureConstants::RegisterCount; ++i) {
			iris::decodeUint16LE(gpr[i], (byte*)wordBuf);
			stream.write(wordBuf, sizeof(word));
		}
		for (auto i = 0; i < ArchitectureConstants::AddressMax; ++i) {
			iris::decodeUint16LE(data[i], (byte*)wordBuf);
			stream.write(wordBuf, sizeof(word));
		}
		for (auto i = 0; i < ArchitectureConstants::AddressMax; ++i) {
			iris::decodeUint32LE(instruction[i], (byte*)dwordBuf);
			stream.write(dwordBuf, sizeof(dword));
		}
		for (auto i = 0; i < ArchitectureConstants::AddressMax; ++i) {
			iris::decodeUint16LE(stack[i], (byte*)wordBuf);
			stream.write(wordBuf, sizeof(word));
		}
	}
	void Core::run() {
		while(execute) {
			if (!advanceIp) {
				advanceIp = true;
			}
			current = instruction[gpr[ArchitectureConstants::InstructionPointerIndex]];
			dispatch();
			if (advanceIp) {
				++gpr[ArchitectureConstants::InstructionPointerIndex];
			}
		}
	}
	void Core::dispatch() {
		auto group = static_cast<InstructionGroup>(getGroup());
#define X(name, operation) \
		if (group == InstructionGroup:: name) { \
			operation(); \
			return; \
		}
#include "def/iris16/groups.def"
#undef X
		std::stringstream stream;
		stream << "Illegal instruction group " << getGroup();
		execute = false;
		throw iris::Problem(stream.str());
	}

	void Core::compare() {
		auto cop = static_cast<CompareOp>(getOperation());
#define X(type, compare, mod) \
			if (cop == CompareOp:: type) { \
				gpr[getDestination()] = (gpr[getSource0()] compare gpr[getSource1()]); \
				return; \
			}
#define Y(type, compare, mod) \
			if (cop == CompareOp:: type) { \
				gpr[getDestination()] = (gpr[getSource0()] compare static_cast<word>(getSource1())); \
				return; \
			}
#include "def/iris16/compare.def"
#undef X
#undef Y
		std::stringstream stream;
		stream << "Illegal compare code " << getOperation();
		execute = false;
		advanceIp = false;
		throw iris::Problem(stream.str());
	}

	void Core::arithmetic() {
		auto operation = static_cast<ArithmeticOp>(getOperation());
#define XNone(n) (gpr[getSource0()], gpr[getSource1()])
#define XImmediate(n) (gpr[getSource0()], static_cast<word>(getSource1()))
#define XUnary(n) (gpr[getSource0()])
#define X(name, op, desc) \
		if (ArithmeticOp:: name == operation) { \
			gpr[getDestination()] = op INDIRECTOR(X, desc)(name); \
			return; \
		}
#include "def/iris16/arithmetic.def"
#undef X
#undef XNone
#undef XDenominator
#undef XUnary
		std::stringstream stream;
		stream << "Illegal arithmetic operation " << getOperation();
		execute = false;
		throw iris::Problem(stream.str());
	}
	template<JumpOp op>
	struct ConditionalStyle {
		static constexpr auto isFalseForm = false;
	};
#define X(name, ifthenelse, conditional, iffalse, immediate, link) \
	template<> struct ConditionalStyle<JumpOp:: name> { static constexpr bool isFalseForm = iffalse; };
#include "def/iris16/jump.def"
#undef X

	void Core::jump() {
		auto newAddr = static_cast<word>(0);
		auto cond = true;
		advanceIp = false;
		auto ip = gpr[ArchitectureConstants::InstructionPointerIndex];
		auto jop = static_cast<JumpOp>(getOperation());
#define XImmediateCond_true (getImmediate())
#define XImmediateCond_false (gpr[getSource0()])
#define XIfThenElse_false(immediate) \
			newAddr = cond ? INDIRECTOR(XImmediateCond, _ ## immediate) : ip + 1;
#define XIfThenElse_true(immediate) \
			newAddr = gpr[cond ? getSource0() : getSource1()];
#define XImmediateUncond_false (gpr[getDestination()])
#define XImmediateUncond_true (getImmediate())
#define XConditional_false(name, ifthenelse, immediate) \
			newAddr = INDIRECTOR(XImmediateUncond, _ ## immediate);
#define XConditional_true(name, ifthenelse, immediate) \
			cond = (ConditionalStyle<JumpOp:: name>::isFalseForm ? (gpr[getDestination()] == 0) : (gpr[getDestination()] != 0)); \
			INDIRECTOR(XIfThenElse, _ ## ifthenelse)(immediate)
#define XLink_true \
			if (cond) { \
				gpr[ArchitectureConstants::LinkRegisterIndex] = ip + 1; \
			}
#define XLink_false
#define X(name, ifthenelse, conditional, iffalse, immediate, link) \
			if (jop == JumpOp:: name) { \
				INDIRECTOR(XConditional, _ ## conditional)(name, ifthenelse, immediate) \
				gpr[ArchitectureConstants::InstructionPointerIndex] = newAddr; \
				INDIRECTOR(XLink, _ ## link)  \
				return; \
			}
#include "def/iris16/jump.def"
#undef X
		std::stringstream ss;
		ss << "Illegal jump code " << getOperation();
		execute = false;
		throw iris::Problem(ss.str());
	}
	void Core::misc() {
		auto op = static_cast<MiscOp>(getOperation());
#define X(name, func) \
		if (op == MiscOp:: name) { \
			func () ; \
			return; \
		}
#include "def/iris16/misc.def"
#undef X
		std::cerr << "Illegal misc code " << getOperation() << std::endl;
		execute = false;
		advanceIp = false;
	}
	void Core::systemCall() {
		auto target = static_cast<SystemCalls>(getDestination());
		if (target == SystemCalls::Terminate) {
			execute = false;
			advanceIp = false;
		} else if (target == SystemCalls::PutC) {
			// read register 0 and register 1
			std::cout.put(static_cast<char>(gpr[getSource0()]));
		} else if (target == SystemCalls::GetC) {
			auto value = static_cast<byte>(0);
			std::cin >> std::noskipws >> value;
			gpr[getSource0()] = static_cast<word>(value);
		} else {
			std::stringstream stream;
			stream << "Illegal system call " << std::hex << getDestination();
			execute = false;
			advanceIp = false;
			throw iris::Problem(stream.str());
		}
	}
	void Core::move() {
		auto a = static_cast<word>(0);
		auto mop = static_cast<MoveOp>(getOperation());
#define GPRRegister0 (gpr[getDestination()])
#define GPRRegister1 (gpr[getSource0()])
#define GPRRegister2 (gpr[getSource1()])
#define GPRImmediate1 (getImmediate())
#define DataRegister0 GPRRegister0
#define DataRegister1 GPRRegister1
#define DataImmediate1 GPRImmediate1
#define StackPushRegister0 (gpr[ArchitectureConstants::StackPointerIndex])
#define StackPushRegister1 GPRRegister0
#define StackPushImmediate1 GPRImmediate1
#define StackPopRegister0 GPRRegister0
#define StackPopRegister1 (gpr[ArchitectureConstants::StackPointerIndex])
#define StackPopImmediate1 GPRImmediate1
#define StoreRegister0  GPRRegister0
#define StoreRegister1 GPRRegister1
#define StoreImmediate1 GPRImmediate1
#define CodeRegister0 GPRRegister0
#define CodeUpperLowerRegisters1 GPRRegister1
#define CodeUpperLowerRegisters2 GPRRegister2
#define XLoadCode(type, dest, src) \
			auto result = instruction[INDIRECTOR(type, dest ## 0)]; \
			INDIRECTOR(type, src ## 1) = static_cast<word>(result); \
			INDIRECTOR(type, src ## 2) = static_cast<word>(result >> 16);
#define XStoreCode(type, dest, src) \
			instruction[INDIRECTOR(type, dest ## 0)] = (((static_cast<dword>(INDIRECTOR(type, src ## 2))) << 16) | (static_cast<dword>(INDIRECTOR(type, src ## 1))));
#define XMove(type, dest, src) \
			INDIRECTOR(type, dest ## 0) = INDIRECTOR(type, src ## 1);
#define XSwap(type, dest, src) \
			a = INDIRECTOR(type, dest ##  0); \
			INDIRECTOR(type, dest ## 0) = INDIRECTOR(type, src ## 1); \
			INDIRECTOR(type, src ##  1) = a;
#define XLoad(type, dest, src) \
			INDIRECTOR(type, dest ## 0) = data[INDIRECTOR(type, src ## 1)];
#define XPop(type, dest, src) \
			INDIRECTOR(type, Pop ## dest ## 0) = stack[INDIRECTOR(type, Pop ## src ## 1)]; \
			--INDIRECTOR(type, Pop ## src ## 1);
#define XPush(type, dest, src) \
			++INDIRECTOR(type, Push ## dest ## 0); \
			stack[INDIRECTOR(type, Push ## dest ## 0)] = INDIRECTOR(type, Push ## src ## 1);
#define XStore(type, dest, src) \
			data[INDIRECTOR(type, dest ##  0)] = INDIRECTOR(type, src ## 1);
#define X(name, type, target, dest, src) \
		if (MoveOp:: name == mop ) { \
			INDIRECTOR(X,type)(target, dest, src) \
			return; \
		}
#include "def/iris16/move.def"
#undef X
#undef XMove
#undef XSwap
#undef XLoad
#undef XStore
#undef XPop
#undef XPush
#undef GPRRegister0
#undef GPRRegister1
#undef GPRImmediate1
#undef DataRegister0
#undef DataRegister1
#undef DataImmediate1
#undef StackPushRegister0
#undef StackPushRegister1
#undef StackPushImmediate1
#undef StackPopRegister0
#undef StackPopRegister1
#undef StackPopImmediate1
#undef StoreRegister0
#undef StoreRegister1
#undef StoreImmediate1
#undef XStoreCode
#undef XLoadCode
#undef CodeRegister0
#undef CodeUpperLowerRegisters1
#undef CodeUpperLowerRegisters2
		std::stringstream ss;
		ss << "Illegal move code " << getOperation();
		execute = false;
		advanceIp = false;
		throw iris::Problem(ss.str());
	}

	enum class Segment  {
		Code,
		Data,
		Count,
	};
	void Core::link(std::istream& input) {
		char buf[8] = {0};
		for(auto lineNumber = static_cast<int>(0); input.good(); ++lineNumber) {
			input.read(buf, 8);
			if (input.gcount() < 8 && input.gcount() > 0) {
				throw iris::Problem("unaligned object file found!");
			} else if (input.gcount() == 0) {
				if (input.eof()) {
					break;
				} else {
					throw iris::Problem("Something bad happened while reading input file!");
				}
			}
			//ignore the first byte, it is always zero
			auto target = static_cast<Segment>(buf[1]);
			auto address = iris16::encodeWord(buf[2], buf[3]);
			if (debugEnabled()) {
				std::cerr << "current target = " << static_cast<int>(target) << "\tcurrent address = 0x" << std::hex << address << std::endl;
			}
			if (target == Segment::Code) {
				auto result = iris16::encodeDword(buf[4], buf[5], buf[6], buf[7]);
				if (debugEnabled()) {
					std::cerr << " code result: 0x" << std::hex << result << std::endl;
				}
				setInstructionMemory(address, result);
			} else if (target == Segment::Data) {
				auto result = iris16::encodeWord(buf[4], buf[5]);
				if (debugEnabled()) {
					std::cerr << " data result: 0x" << std::hex << result << std::endl;
				}
				setDataMemory(address, result);
			} else {
				std::stringstream str;
				str << "error: line " << lineNumber << ", unknown segment " << static_cast<int>(target) << "/" << static_cast<int>(buf[1]) << std::endl;
				str << "current address: " << std::hex << address << std::endl;
				throw iris::Problem(str.str());
			}
		}
	}
}
