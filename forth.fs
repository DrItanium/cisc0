\ basic string routines
\ enable-debug
\ always pass on the stack
: strlen ( -- n ) temp ;
: 64k ( -- n ) 0x10000 ;
: -64k ( -- n ) 64k - ;
: @-64k ( v -- n ) @ -64k ;
: !$@-64k ( v -- n ) !$@ -64k ;
: !$@1- ( v -- n ) !$@ 1- ;
: @1- ( v -- n ) @ 1- ;

\ 0xFF000 - 0x100000 unused
\ 0xFE000 - 0xFEFFF vmstack
\ 0xFD000 - 0xFDFFF parameter stack
\ 0xFC000 - 0xFCFFF subroutine stack
\ 0xFB000 - 0xFBFFF vm data
\ 0xFB000 - 0xFB0FF input buffer
\ 0x50000 - 0xEFFFF dictionary
0x100000 variable! Capacity
0xFEFFF variable! VMStackEnd
0xFE000 variable! VMStackBegin
0xFE000 variable! ParameterStackBegin
0xFD000 variable! ParameterStackEnd
0xFD000 variable! SubroutineStackBegin
0xFC000 variable! SubroutineStackEnd
0xFC000 variable! VMDataEnd
0xFB000 variable! VMDataStart
VMDataStart variable@! InputBufferStart
InputBufferStart @ 0x100 + variable! InputBufferEnd
InputBufferEnd variable@! VMVariableStart 
VMVariableStart variable@! VMVariablesEnd
0x50000 variable! DictionaryStart
0xF0000 variable! DictionaryEnd
: store-current-end ( variable -- ) 
  VMVariablesEnd @ swap ! ;
: advance-var-end ( count -- )
  VMVariablesEnd @ + VMVariablesEnd ! ;
: sysvar ( -- ) variable$ store-current-end 2 advance-var-end ;
: setvar ( value variable -- ) .orgv .data32 ;
: setvarv ( variable variable ) .orgv .data32v ;
\ variables to define
254 variable! StringInputMax 
sysvar &Capacity
sysvar &IgnoreInput
sysvar &IsCompiling
sysvar &StringInputMax
sysvar &InputBufferStart
sysvar &DictionaryFront
sysvar &ParameterStackEmpty
sysvar &ParameterStackFull
sysvar &SubroutineStackEmpty
sysvar &SubroutineStackFull
sysvar &VMStackEmpty
sysvar &VMStackFull
sysvar &DictionaryStart
sysvar &DictionaryEnd


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
DictionaryStart &DictionaryStart setvarv
DictionaryEnd &DictionaryEnd setvarv
Capacity @ .capacity

: .char ( code -- ) 1 .data32 .data16 ;
variable0! CurrentDictionaryFront
variable0! OldDictionaryFront
DictionaryStart variable@! NextDictionaryEntry
: flag-none ( -- n ) 0x0 ;
: flag-compile-time-invoke ( -- n ) 0x1 ;
: flag-no-more ( -- n ) 0x2 ;
\ Dictionary Format is:
\ 0: Flags
\ 2: Next Entry in Dictionary
\ 4: Name of Entry 
\ 6: Address of code body 
: val<> ( top -- f ) dup val <> ;
: addr<> ( top -- f ) dup addr <> ;
: ->addr? ( top -- ) addr<> if ->addr else drop then ;
: <-addr? ( top -- ) addr<> if <-addr else drop then ;
: <-val? ( top -- ) val<> if <-val else drop then ;
: ->val? ( top -- ) val<> if ->val else drop then ;
: <-dflags ( dest src -- ) ->addr? 0 !load32 <-val? ;
: <-dnext  ( dest src -- ) ->addr? 2 !load32 <-val? ;
: <-dname  ( dest src -- ) ->addr? 4 !load32 <-val? ;
: <-dcode  ( dest src -- ) ->addr? 6 !load32 <-val? ;



\ StringCacheStart .orgv
\ .label NULLSTRING is-here 0x00 .char
\ .label StringOpenParen is-here 0x28 .char
\ .label StringCloseParen is-here 0x29 .char
\ .label StringSpace is-here 0x20 .char
\ .label StringNewline is-here 0xA .char
\ 0 NULLSTRING @ flag-no-more .dictionary-entry 

\ variable0! CurrentVariableCacheStart
\ variable0! OldVariableCacheStart
\ VariableStart variable@! NextVariableCacheStart

\ variable format is:
\ 0: Name { Address }
\ 2: Value 
\ 4: Next Variable 
\ : .variable-entry ( value string -- )
\   NextVariableCacheStart .orgv
\   CurrentVariableCacheStart OldVariableCacheStart @!
\   NextVariableCacheStart CurrentVariableCacheStart @!
\   .data32 
\   .data32
\   OldVariableCacheStart @ .data32 \ store the next pointer
\   NextVariableCacheStart is-here ;

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

\ CodeCacheStart .orgv
\ CodeCacheStart variable@! CurrentCodeCacheStart
\ \ redefine func: to also setup the code location pointers too
\ : func: ( variable -- ) 
\   CodeCacheStart .orgv \ make sure we are in the right spot
\   func: \ now do the old action
\   ;
\ : func; ( -- ) 
\   func; \ output the return
\   CodeCacheStart is-here \ update the code cache pointer too
\   ;
\ now the use of func: and func; will yield data being placed in the code 
\ section as a way to describe the action itself
0 .org

func: LeaveFunctionEarly func;
func: PrintNewline !put-cr func;
func: PrintSpace !put-space func;
func: GetInputBuffer InputBufferStart @ !push-immediate32 func;
func: GetStringLength
    ( a -- length )
    \ using we need to load the first 32-bit cell into the register
    addr !popr 
    0 !load32
    val !pushr
func;
func: ComputeStringStart
    ( a -- addr )
    addr !popr
    addr !2+
    addr !pushr
func;
func: PrintString
    ( a -- )
    !dup \ make a copy of the address
    GetStringLength !cuv
    !swap 
    ComputeStringStart !cuv
    addr !popr \ then load the string start into addr
    temp !popr \ load the string length into temp
    temp !0=  \ see if temp is zero
    !leave-on-true \ get out of here if it is not zero
func: PrintStringLoopTop
    0 !load8 \ load the character
    val !putc
    addr !1+
    temp !1-
    temp !0=
    PrintStringLoopTop !jcv
func;

func: PrintLine
    ( a -- )
    PrintString !cuv
    !put-cr 
    func;
func: ReadChar 
    val !getc
    val !push32
    func;
func: ReadWord
    254 temp !set8
    InputBufferStart @ temp2 !set32
    temp temp2 !read-word
    func;
func: StoreToMemory
    ( value address -- )
    addr !popr
    val !popr
    0 !store32
    func;
: save-register-to-address ( reg addr -- )
  addr !set32 
  ->val?
  0 !store32 ;
: save-register-to-variable ( reg var -- ) @ save-register-to-address ;

func: SynchronizeRegisterWithVariables
    \ strp &CurrentStringCacheStart save-register-to-variable 
    \ vp &CurrentVariableCacheStart save-register-to-variable 
    \ codp &CurrentCodeCacheStart save-register-to-variable
    dp &DictionaryFront save-register-to-variable 
    func;

VMStackEnd vmsp .registerv
ParameterStackEnd sp   .registerv
SubroutineStackEnd subrp  .registerv
\ CurrentCodeCacheStart codp .registerv
\ CurrentVariableCacheStart vp .registerv
\ CurrentStringCacheStart strp .registerv
CurrentDictionaryFront dp .registerv
\ CurrentCodeCacheStart &CurrentCodeCacheStart setvarv
\ CurrentStringCacheStart &CurrentStringCacheStart setvarv
CurrentDictionaryFront &DictionaryFront setvarv
\ CurrentVariableCacheStart &CurrentVariableCacheStart setvarv
asm}
close-input-file
