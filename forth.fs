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
variable VMVariablesStart
variable VMVariableEnd
: store-current-end ( variable -- ) 
  VMVariableEnd @ swap ! ;
: defvar16 ( variable -- ) 
  store-current-end 
  VMVariableEnd @ 1+ VMVariableEnd ! ;
: defvar32 ( variable -- ) 
  store-current-end 
  VMVariableEnd @ 2+ VMVariableEnd ! ;
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
InputBufferEnd @ VMVariablesStart !
VMVaraiablesStart @ VMVariablesEnd !

InputBufferStart @ 1- DictionaryEnd ! 
DictionaryEnd @ 0x3FFFFF - DictionaryStart !
DictionaryStart @ 1- StringCacheEnd !
StringCacheEnd @ 0x3FFFFF - StringCacheStart !
StringCacheStart @ 1- CodeCacheEnd !
CodeCacheEnd @ 0x3FFFFF - CodeCacheStart !
\ variables to define
: setvar16 ( value variable -- ) .orgv .data16 ;
: setvar32 ( value variable -- ) .orgv .data32 ;
variable &IgnoreInput
variable &IsCompiling
variable &Capacity
&IgnoreInput defvar16
&IsCompiling defvar16
&Capacity defvar32

false &IgnoreInput setvar16
false &IsCompiling setvar16
Capacity @ &Capacity setvar32


variable LeaveFunctionEarly
variable NeedLocals
variable DoneWithLocals
: !newline? ( reg -- ) 0xA swap !eqi8 ;
: !space? ( reg -- ) 0x20 swap !eqi8 ;
: !)? ( reg -- ) 0x29 swap !eqi8 ;
: !cuv ( variable -- ) @ !cu ;
: !jcv ( variable -- ) @ !jc ;
: !juv ( variable -- ) @ !ju ;
: !leave-on-true ( -- ) LeaveFunctionEarly !jcv ;
: !need-locals ( -- ) NeedLocals !cuv ;
: !restore-locals ( -- ) DoneWithLocals !cuv ;
{asm
Capacity @ .capacity


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
    temp !popr \ load the string length into temp
    temp !0=  \ see if temp is zero
    !leave-on-true \ get out of here if it is not zero
.label PrintStringLoopTop is-here
    0 !load8 \ load the character
    val !putc
    addr !1+
    temp !1-
    temp !0=
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
.label ReadWord func:
    InputBufferStart @ strp !set32
    254 temp !set8
    temp strp !read-word 
    func;
.label IgnoreInput func:
    
: .char ( code -- ) 1 .data32 .data16 ;
variable CurrentDictionaryFront
variable OldDictionaryFront
0 CurrentDictionaryFront !
0 OldDictionaryFront !
: flag-none ( -- n ) 0x0 ;
: flag-fake ( -- n ) 0x1 ;
: flag-compile-time-invoke ( -- n ) 0x2 ;
: flag-no-more ( -- n ) 0x4 ;
: .dictionary-entry ( code string flags -- ) 
  CurrentDictionaryFront @ OldDictionaryFront !
  CurrentDictionaryFront is-here
  .data32 
  OldDictionaryFront @ .data32 \ get the previous entry
  .data32 
  .data32 ;
StringCacheStart .orgv
.label NULLSTRING is-here 0x00 .char
.label StringOpenParen is-here 0x28 .char
.label StringCloseParen is-here 0x29 .char
.label StringSpace is-here 0x20 .char
.label StringNewline is-here 0xA .char
DictionaryStart .orgv
0 NULLSTRING @ flag-no-more .dictionary-entry 

VMStackEnd @ vmsp .register
ParameterStackEnd @ sp   .register
SubroutineStackEnd @ subrp  .register
CodeCacheStart @ codp .register
StringCacheStart @ strp .register
CurrentDictionaryFront @ dp   .register
asm}
close-input-file
