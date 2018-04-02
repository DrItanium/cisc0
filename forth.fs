\ basic string routines
\ always pass on the stack
: strlen ( -- n ) temp ;
variable LeaveFunctionEarly
variable ComputeStringStart
variable GetStringLength
variable PrintString
: !newline? ( reg -- ) 0xA swap !eqi8 ;
: !space? ( reg -- ) 0x20 swap !eqi8 ;
: !leave-on-true ( -- ) LeaveFunctionEarly @ !jc ;
{asm
LeaveFunctionEarly func: func;
GetStringLength func:
    ( a -- length )
    \ using we need to load the first 32-bit cell into the register
    addr !popr 
    0 !load32
    val !pushr
func;
ComputeStringStart func:
    ( a -- addr )
    addr !popr
    addr !2+
    addr !pushr
func;
PrintString func:
.label PrintStringLoopTop 
    ( a -- )
    !dup \ make a copy of the address
    GetStringLength @ !cu
    !!swap 
    ComputeStringStart @ !cu
    addr !popr \ then load the string start into addr
    strlen !popr \ load the string length into temp
    strlen !0=  \ see if temp is zero
    !leave-on-true \ get out of here if it is not zero
    PrintStringLoopTop is-here 
    0 !load8 \ load the character
    val !putc
    addr !1+
    strlen !1-
    strlen !0=
    PrintStringLoopTop @ !jc 
func;

asm}
close-input-file
