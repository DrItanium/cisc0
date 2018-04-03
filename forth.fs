\ basic string routines
\ enable-debug
\ always pass on the stack
: strlen ( -- n ) temp ;
: 64k ( -- n ) 0x10000 ;
: @-64k ( v -- n ) @ 64k - ;
variable Capacity
variable VMStackEnd
variable VMStackBegin
variable ParameterStackBegin
variable ParameterStackEnd
variable SubroutineStackBegin
variable SubroutineStackEnd
variable VMDataStart
variable VMDataEnd
variable InputBufferStart
variable InputBufferEnd
variable DictionaryStart
variable DictionaryEnd 
variable StringCacheStart 
variable StringCacheEnd
variable CodeCacheStart
variable CodeCacheEnd
\ 0xFE0000 - 0xFEFFFF vmstack
\ 0xFD0000 - 0xFDFFFF parameter stack
\ 0xFC0000 - 0xFCFFFF subroutine stack
\ 0xFB0000 - 0xFBFFFF vm data
\ 0xFB0000 - 0xFB00FF input buffer
\ 0xD00000 - 0xEFFFFF dictionary
\ 0xA00000 - 0xCFFFFF strings
\ 0x600000 - 0x9FFFFF code
0x100000 Capacity !
Capacity @-64k VMStackEnd ! 
VMStackEnd @-64k VMStackBegin !
VMStackBegin @ ParameterStackEnd !
ParameterStackEnd @-64k ParameterStackBegin !
ParameterStackBegin @ SubroutineStackEnd !
SubroutineStackEnd @-64k SubroutineStackBegin !
SubroutineStackBegin @ 1- VMDataEnd !
VMDataEnd @ 0xFFFF - VMDataStart !
VMDataStart @ InputBufferStart !
0x100 InputBufferStart @ + InputBufferEnd !
InputBufferStart @ 1- DictionaryEnd ! 
DictionaryEnd @ 0x3FFFFF - DictionaryStart !
DictionaryStart @ 1- StringCacheEnd !
StringCacheEnd @ 0x3FFFFF - StringCacheStart !
StringCacheStart @ 1- CodeCacheEnd !
CodeCacheEnd @ 0x3FFFFF - CodeCacheStart !

variable LeaveFunctionEarly
variable NeedLocals
variable DoneWithLocals
: !newline? ( reg -- ) 0xA swap !eqi8 ;
: !space? ( reg -- ) 0x20 swap !eqi8 ;
: !cuv ( variable -- ) @ !cu ;
: !jcv ( variable -- ) @ !jc ;
: !leave-on-true ( -- ) LeaveFunctionEarly !jcv ;
: !need-locals ( -- ) NeedLocals !cuv ;
: !restore-locals ( -- ) DoneWithLocals !cuv ;
{asm
0x1000000 .capacity
VMStackEnd @ vmsp .register
ParameterStackEnd @ sp   .register
SubroutineStackEnd @ subrp  .register
DictionaryStart @ dp   .register
CodeCacheStart @ codp .register
StringCacheStart @ strp .register


0x0000100 .org 
LeaveFunctionEarly func: func;
NeedLocals func:
{vmstack 
    loc0 !pushr
    loc1 !pushr
    loc2 !pushr
    loc3 !pushr
vmstack}
func;
DoneWithLocals func:
{vmstack 
    loc3 !popr
    loc2 !popr
    loc1 !popr
    loc0 !popr
vmstack}
func;
.label PrintNewline func: !put-cr func;
.label PrintSpace func: !put-space func;
.label GetInputBuffer func: InputBufferStart @ !push-immediate32 func;
.label GetStringLength func:
    ( a -- length )
    \ using we need to load the first 32-bit cell into the register
    addr !popr 
    0 !load32
    val !pushr
func;
.label ComputeStringStart func:
    ( a -- addr )
    addr !popr
    addr !2+
    addr !pushr
func;
.label PrintString func:
    ( a -- )
    !dup \ make a copy of the address
    GetStringLength !cuv
    !swap 
    ComputeStringStart !cuv
    addr !popr \ then load the string start into addr
    strlen !popr \ load the string length into temp
    strlen !0=  \ see if temp is zero
    !leave-on-true \ get out of here if it is not zero
.label PrintStringLoopTop is-here
    0 !load8 \ load the character
    val !putc
    addr !1+
    strlen !1-
    strlen !0=
    PrintStringLoopTop !jcv
func;

.label PrintLine func:
    ( a -- )
    PrintString !cuv
    !put-cr 
    func;
.label ReadChar func:
    val !getc
    val !push32
    func;

: base-address ( -- n ) loc0 ;
: str-length ( -- n ) loc1 ;
.label ReadLine func:
    !need-locals
    
    !restore-locals
func;

asm}
close-input-file
