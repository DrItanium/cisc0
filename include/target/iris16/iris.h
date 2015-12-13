#ifndef _TARGET_IRIS16_IRIS_H
#define _TARGET_IRIS16_IRIS_H
#include "iris_base.h"
#include "Core.h"
#include <cstdint>
typedef uint16_t word;
typedef uint32_t dword;
typedef dword raw_instruction;
typedef word immediate;
namespace iris16 {
	enum ArchitectureConstants  {
		RegisterCount = 256,
		AddressMax = 65535,
		InstructionPointerIndex = 255,
		LinkRegisterIndex = 254,
		StackPointerIndex = 253,
		MaxGroups = 0b111,
		MaxOperations = 0b11111,
	};
	dword encodeDword(byte a, byte b, byte c, byte d);
	word encodeWord(byte a, byte b);

	enum class InstructionGroup : byte {
#define X(title, _, __) title,
#include "target/iris16/groups.def"
#undef X
		Count,
	};
	static_assert((byte)InstructionGroup::Count < ArchitectureConstants::MaxGroups, "too many instruction groups defined");
	enum class ArithmeticOp : byte {
#define X(name, _, __, ___) name,
#include "target/iris16/arithmetic.def"
#undef X
		Count
	};
	static_assert((byte)ArithmeticOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Arithmetic operations defined");
	enum class MiscOp : byte {
#define X(title, id, func) title, 
#include "target/iris16/misc.def"
#undef X
		Count
	};
	static_assert((byte)MiscOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Misc operations defined");

	enum class JumpOp : byte {
#define X(name, id, ifthenelse, conditional, iffalse, immediate, link) name,
#include "target/iris16/jump.def"
#undef X
		Count
	};
	static_assert((byte)JumpOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Jump operations defined");

	enum class SystemCalls : byte {
#define X(name) name,
#include "target/iris16/syscalls.def"
#undef X
		Count,
	};
	enum class MoveOp : byte {
#define X(name, id, type, target, dest, src) name,
#include "target/iris16/move.def"
#undef X
		Count,
	};
	static_assert((byte)MoveOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Move operations defined");
	enum class CompareOp : byte {
#define X(name, id, op, group) name,
#include "target/iris16/compare.def"
#undef X
		Count,
	};
	static_assert((byte)CompareOp::Count < ((byte)ArchitectureConstants::MaxOperations), "too many Compare operations defined");
	class DecodedInstruction {
		public:
			DecodedInstruction();
			void decode(raw_instruction input);
#define X(title, mask, shift, type, is_register, post) type get ## title () const { return _ ## post ; }
#include "target/iris16/instruction.def"
#undef X
		private:
#define X(title, mask, shift, type, is_register, post) type _ ## post ;
#include "target/iris16/instruction.def"
#undef X
	};
	class Core : public iris::Core {
		public:
			Core();
			virtual void initialize();
			virtual void installprogram(std::istream& stream);
			virtual void shutdown();
			virtual void dump(std::ostream& stream);
			virtual void run();
			void setInstructionMemory(word address, dword value);
			void setDataMemory(word address, word value);
		private:
			void dispatch();
#define X(_, op, __) void op();
#include "target/iris16/groups.def"
#undef X
#define X(title, id, func) void func ();
#include "target/iris16/misc.def"
#undef X

		private:
			DecodedInstruction current;
			bool execute = true,
				 advanceIp = true;
			word gpr[ArchitectureConstants::RegisterCount] = {0};
			word data[ArchitectureConstants::AddressMax] = { 0 };
			dword instruction[ArchitectureConstants::AddressMax] = { 0 };
			word stack[ArchitectureConstants::AddressMax] = { 0 };
	};

}
#endif
