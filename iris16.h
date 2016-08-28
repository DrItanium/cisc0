#ifndef _TARGET_IRIS16_IRIS_H
#define _TARGET_IRIS16_IRIS_H
#include "iris_base.h"
#include "Core.h"
#include <cstdint>
namespace iris16 {
	typedef uint16_t word;
	typedef uint32_t dword;
	typedef dword raw_instruction;
	typedef word immediate;
	enum ArchitectureConstants  {
		RegisterCount = 256,
		AddressMax = 65535,
		InstructionPointerIndex = RegisterCount - 1,
		LinkRegisterIndex = RegisterCount - 2,
		StackPointerIndex = RegisterCount - 3,
		MaxGroups = 0x7,
		MaxOperations = 0x1F,
	};
	inline constexpr dword encodeDword(byte a, byte b, byte c, byte d) noexcept {
		return iris::encodeUint32LE(a, b, c, d);
	}
	inline constexpr word encodeWord(byte a, byte b) noexcept {
		return iris::encodeUint16LE(a, b);
	}
	inline constexpr dword encodeDword(word lower, word upper) noexcept {
		return iris::encodeBits<word, dword, 0xFFFF0000, 16>(iris::encodeBits<word, dword, 0x0000FFFF, 0>(0, lower), upper);
	}

	enum class InstructionGroup : byte {
#define X(title, _) title,
#include "def/iris16/groups.def"
#undef X
		Count,
	};
	static_assert((byte)InstructionGroup::Count < ((byte)ArchitectureConstants::MaxGroups), "too many instruction groups defined");
	enum class ArithmeticOp : byte {
#define X(name, __, ___) name,
#include "def/iris16/arithmetic.def"
#undef X
		Count
	};
	static_assert((byte)ArithmeticOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Arithmetic operations defined");
	enum class MiscOp : byte {
#define X(title, func) title, 
#include "def/iris16/misc.def"
#undef X
		Count
	};
	static_assert((byte)MiscOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Misc operations defined");

	enum class JumpOp : byte {
#define X(name, ifthenelse, conditional, iffalse, immediate, link) name,
#include "def/iris16/jump.def"
#undef X
		Count
	};
	static_assert((byte)JumpOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Jump operations defined");

	enum class SystemCalls : byte {
#define X(name) name,
#include "def/iris16/syscalls.def"
#undef X
		Count,
	};
	enum class MoveOp : byte {
#define X(name, type, target, dest, src) name,
#include "def/iris16/move.def"
#undef X
		Count,
	};
	static_assert((byte)MoveOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Move operations defined");
	enum class CompareOp : byte {
#define X(name, op, group) name,
#define Y(name, op, group) name,
#include "def/iris16/compare.def"
#undef Y
#undef X
		Count,
	};
	static_assert((byte)CompareOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Compare operations defined");
	class Core : public iris::Core {
		public:
			Core() noexcept { }
			virtual ~Core();
			virtual void initialize() override { }
			virtual void installprogram(std::istream& stream) override;
			virtual void shutdown() override { }
			virtual void dump(std::ostream& stream) override;
			virtual void run() override;
			virtual void link(std::istream& input) override;
		private:
			inline void setInstructionMemory(word address, dword value) noexcept;
			inline void setDataMemory(word address, word value) noexcept;
		private:
			void dispatch();
			template<bool ifthenelse, bool conditional, bool iffalse, bool immediate, bool link>
			inline void jumpBody() noexcept {
				auto newAddr = static_cast<word>(0);
				auto cond = true;
				advanceIp = false;
				auto ip = gpr[ArchitectureConstants::InstructionPointerIndex];
				if (conditional) {
					auto dest = gpr[getDestination()];
					cond = (iffalse ? (dest == 0) : (dest != 0));
					if (ifthenelse) {
						newAddr = gpr[cond ? getSource0() : getSource1()];
					} else {
						newAddr = cond ? (immediate ? getImmediate() : gpr[getSource0()]) : ip + 1;
					}
				} else {
					newAddr = immediate ? getImmediate() : gpr[getDestination()];
				}
				gpr[ArchitectureConstants::InstructionPointerIndex] = newAddr;
				if (link && cond) {
					gpr[ArchitectureConstants::LinkRegisterIndex] = ip + 1;
				}
			}
			template<MoveOp op>
			inline void moveBody() {
				throw iris::Problem("Undefined move operation");
			}
#define X(_, op) void op();
#include "def/iris16/groups.def"
#undef X
#define X(title, func) void func ();
#include "def/iris16/misc.def"
#undef X
#define X(title, mask, shift, type, is_register, post) \
			inline type get ## title () const noexcept { \
				return iris::decodeBits<raw_instruction, type, mask, shift>(current); \
			} 
#include "def/iris16/instruction.def"
#undef X
		private:
			raw_instruction current;
			bool execute = true,
				 advanceIp = true;
			word gpr[ArchitectureConstants::RegisterCount] = {0};
			word data[ArchitectureConstants::AddressMax] = { 0 };
			dword instruction[ArchitectureConstants::AddressMax] = { 0 };
			word stack[ArchitectureConstants::AddressMax] = { 0 };
	};

	template<>
	inline void Core::moveBody<MoveOp::Move>() {
		gpr[getDestination()] = gpr[getSource0()];
	}

	template<>
	inline void Core::moveBody<MoveOp::Set>() {
		gpr[getDestination()] = static_cast<word>(getImmediate());
	}

	template<>
	inline void Core::moveBody<MoveOp::Swap>() {
		auto result = instruction[gpr[getDestination()]];
		instruction[gpr[getDestination()]] = instruction[gpr[getSource0()]];
		instruction[gpr[getSource0()]] = result;
	}

	template<>
	inline void Core::moveBody<MoveOp::Load>() {
		gpr[getDestination()] = data[gpr[getSource0()]];
	}

	template<>
	inline void Core::moveBody<MoveOp::LoadImmediate>() {
		gpr[getDestination()] = data[getImmediate()];
	}

	template<>
	inline void Core::moveBody<MoveOp::Store>() {
		data[gpr[getDestination()]] = gpr[getSource0()];
	}

	template<>
	inline void Core::moveBody<MoveOp::Memset>() {
		data[gpr[getDestination()]] = getImmediate();
	}

	template<>
	inline void Core::moveBody<MoveOp::Push>() {
		++gpr[ArchitectureConstants::StackPointerIndex];
		stack[gpr[ArchitectureConstants::StackPointerIndex]] = gpr[getDestination()];
	}

	template<>
	inline void Core::moveBody<MoveOp::PushImmediate>() {
		++gpr[ArchitectureConstants::StackPointerIndex];
		stack[gpr[ArchitectureConstants::StackPointerIndex]] = getImmediate();
	}

	template<>
	inline void Core::moveBody<MoveOp::Pop>() {
		gpr[getDestination()] = stack[gpr[ArchitectureConstants::StackPointerIndex]];
		--gpr[ArchitectureConstants::StackPointerIndex];
	}

	template<>
	inline void Core::moveBody<MoveOp::LoadCode>() {
		auto result = instruction[gpr[getDestination()]];
		gpr[getSource0()] = iris::decodeBits<dword, word, 0x0000FFFF, 0>(result);
		gpr[getSource1()] = iris::decodeBits<dword, word, 0xFFFF0000, 16>(result);
	}

	template<>
	inline void Core::moveBody<MoveOp::StoreCode>() {
		instruction[getDestination()] = encodeDword(gpr[getSource0()], gpr[getSource1()]);
	}

	Core* newCore() noexcept;
}
#endif
