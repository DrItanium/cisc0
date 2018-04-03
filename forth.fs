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
variable VMVariablesEnd
variable VariableStart
variable VariableEnd
: store-current-end ( variable -- ) 
  VMVariablesEnd @ swap ! ;
: advance-var-end ( count -- )
  VMVariablesEnd @ + VMVariablesEnd ! ;
: defvar ( variable -- ) store-current-end 2 advance-var-end ;
: setvar ( value variable -- ) .orgv .data32 ;
: setvarv ( variable variable ) .orgv .data32v ;
\ 0xFE0000 - 0xFEFFFF vmstack
\ 0xFD0000 - 0xFDFFFF parameter stack
\ 0xFC0000 - 0xFCFFFF subroutine stack
\ 0xFB0000 - 0xFBFFFF vm data
\ 0xFB0000 - 0xFB00FF input buffer
\ 0xD00000 - 0xEFFFFF dictionary
\ 0xA00000 - 0xCFFFFF strings
\ 0x600000 - 0x9FFFFF code
\ 0x500000 - 0x5FFFFF variables
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
VMVariablesStart @ VMVariablesEnd !

InputBufferStart @ 1- DictionaryEnd ! 
DictionaryEnd @ 0x3FFFFF - DictionaryStart !
DictionaryStart @ 1- StringCacheEnd !
StringCacheEnd @ 0x3FFFFF - StringCacheStart !
StringCacheStart @ 1- CodeCacheEnd !
CodeCacheEnd @ 0x3FFFFF - CodeCacheStart !
CodeCacheStart @ 1- VariableEnd !
VariableEnd @ 0x0FFFFF - VariableStart !
\ variables to define
variable StringInputMax
254 StringInputMax !
variable &IgnoreInput
variable &IsCompiling
variable &Capacity
variable &StringInputMax
variable &InputBufferStart
variable &DictionaryFront
variable &ParameterStackEmpty 
variable &ParameterStackFull
variable &SubroutineStackEmpty 
variable &SubroutineStackFull
variable &VMStackEmpty 
variable &VMStackFull
variable &StringCacheStart
variable &StringCacheEnd
variable &CodeCacheStart
variable &CodeCacheEnd
variable &DictionaryStart
variable &DictionaryEnd
variable &VariableStart
variable &VariableEnd
variable &CurrentStringCacheStart
variable &CurrentCodeCacheStart
variable &CurrentVariableCacheStart
&Capacity defvar
&IgnoreInput defvar
&IsCompiling defvar
&StringInputMax defvar
&InputBufferStart defvar
&DictionaryFront defvar
&ParameterStackEmpty defvar
&ParameterStackFull defvar
&SubroutineStackEmpty defvar
&SubroutineStackFull defvar
&VMStackEmpty defvar
&VMStackFull defvar
&StringCacheStart defvar
&StringCacheEnd defvar
&CodeCacheStart defvar
&CodeCacheEnd defvar
&DictionaryStart defvar
&DictionaryEnd defvar
&VariableStart defvar
&VariableEnd defvar
&CurrentStringCacheStart defvar
&CurrentCodeCacheStart defvar
&CurrentVariableCacheStart defvar


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
{asm
false &IgnoreInput setvar
false &IsCompiling setvar
Capacity &Capacity setvarv
StringInputMax &StringInputMax setvarv
InputBufferStart &InputBufferStart setvarv
VMStackEnd &VMStackEmpty setvarv
VMStackBegin &VMStackFull setvarv
ParameterStackEnd &ParameterStackEmpty setvarv
ParameterStackBegin &ParameterStackFull setvarv
SubroutineStackEnd &SubroutineStackEmpty setvarv
SubroutineStackBegin &SubroutineStackFull setvarv
StringCacheStart &StringCacheStart setvarv
StringCacheEnd &StringCacheEnd setvarv
CodeCacheStart &CodeCacheStart setvarv
CodeCacheEnd &CodeCacheEnd setvarv
DictionaryStart &DictionaryStart setvarv
DictionaryEnd &DictionaryEnd setvarv
VariableStart &VariableStart setvarv
VariableEnd &VariableEnd setvarv
Capacity @ .capacity

: .char ( code -- ) 1 .data32 .data16 ;
variable CurrentDictionaryFront
variable OldDictionaryFront
variable NextDictionaryEntry 
CurrentDictionaryFront 0!
OldDictionaryFront 0!
DictionaryStart @ NextDictionaryEntry !
: flag-none ( -- n ) 0x0 ;
: flag-fake ( -- n ) 0x1 ;
: flag-compile-time-invoke ( -- n ) 0x2 ;
: flag-no-more ( -- n ) 0x4 ;
: .dictionary-entry ( code string flags -- ) 
  NextDictionaryEntry .orgv
  CurrentDictionaryFront @ OldDictionaryFront !
  NextDictionaryEntry @ CurrentDictionaryFront !
  .data32 
  OldDictionaryFront @ .data32 \ get the previous entry
  .data32 
  .data32 
  NextDictionaryEntry is-here ;
variable CurrentStringCacheStart
StringCacheStart @ CurrentStringCacheStart !
: .char-entry ( char label -- )
  is-here 
  .char
  CurrentStringCacheStart is-here ;

StringCacheStart .orgv
.label NULLSTRING is-here 0x00 .char
.label StringOpenParen is-here 0x28 .char
.label StringCloseParen is-here 0x29 .char
.label StringSpace is-here 0x20 .char
.label StringNewline is-here 0xA .char
DictionaryStart .orgv
0 NULLSTRING @ flag-no-more .dictionary-entry 

VariableStart .orgv
variable CurrentVariableCacheStart
variable OldVariableCacheStart
variable NextVariableCacheStart
CurrentVariableCacheStart 0!
OldVariableCacheStart 0!
VariableStart @ NextVariableCacheStart !

: .variable-entry ( value string -- )
  NextVariableCacheStart .orgv
  CurrentVariableCacheStart @ OldVariableCacheStart !
  NextVariableCacheStart @ CurrentVariableCacheStart !
  .data32 
  .data32
  OldVariableCacheStart @ .data32 \ store the next pointer
  NextVariableCacheStart is-here ;

: !need-locals ( -- )
{vmstack 
    loc0 !pushr
    loc1 !pushr
    loc2 !pushr
    loc3 !pushr
vmstack} ;
: !restore-locals ( -- ) 
{vmstack 
    loc3 !popr
    loc2 !popr
    loc1 !popr
    loc0 !popr
vmstack} ;

CodeCacheStart .orgv
variable CurrentCodeCacheStart
CodeCacheStart @ CurrentCodeCacheStart !
\ redefine func: to also setup the code location pointers too
: func: ( variable -- ) 
  CodeCacheStart .orgv \ make sure we are in the right spot
  func: \ now do the old action
  ;
: func; ( -- ) 
  func; \ output the return
  CodeCacheStart is-here \ update the code cache pointer too
  ;
\ now the use of func: and func; will yield data being placed in the code 
\ section as a way to describe the action itself
0 .org

LeaveFunctionEarly func: func;
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
    254 temp !set8
    InputBufferStart @ temp2 !set32
    temp temp2 !read-word
    func;
.label StoreToMemory func:
    ( value address -- )
    addr !popr
    val !popr
    0 !store32
    func;
: save-register-to-address ( reg addr -- )
  addr !set32 
  dup val <> if ->val else drop then \ don't emit an instruction if register is val already
  0 !store32 ;
: save-register-to-variable ( reg var -- ) @ save-register-to-address ;

.label SynchronizeRegisterWithVariables func:
    strp &CurrentStringCacheStart save-register-to-variable 
    vp &CurrentVariableCacheStart save-register-to-variable 
    codp &CurrentCodeCacheStart save-register-to-variable
    dp &DictionaryFront save-register-to-variable 
    func;

VMStackEnd vmsp .registerv
ParameterStackEnd sp   .registerv
SubroutineStackEnd subrp  .registerv
CurrentCodeCacheStart codp .registerv
CurrentVariableCacheStart vp .registerv
CurrentStringCacheStart strp .registerv
CurrentDictionaryFront dp .registerv
CurrentCodeCacheStart &CurrentCodeCacheStart setvarv
CurrentStringCacheStart &CurrentStringCacheStart setvarv
CurrentDictionaryFront &DictionaryFront setvarv
CurrentVariableCacheStart &CurrentVariableCacheStart setvarv
asm}
close-input-file
