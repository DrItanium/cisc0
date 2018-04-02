" misc/forth_interpreter/basics.fs" open-input-file

{enum
: r0 ( -- n ) literal ; enum,
: r1 ( -- n ) literal ; enum,
: r2 ( -- n ) literal ; enum,
: r3 ( -- n ) literal ; enum,
: r4 ( -- n ) literal ; enum,
: r5 ( -- n ) literal ; enum,
: r6 ( -- n ) literal ; enum,
: r7 ( -- n ) literal ; enum,
: r8 ( -- n ) literal ; enum,
: r9 ( -- n ) literal ; enum,
: r10 ( -- n ) literal ; enum,
: r11 ( -- n ) literal ; enum,
: r12 ( -- n ) literal ; enum,
: r13 ( -- n ) literal ; enum,
: r14 ( -- n ) literal ; enum,
: r15 ( -- n ) literal ; 
enum}

: pc ( -- n ) r15 ;
: sp ( -- n ) r14 ;
: csp ( -- n ) r13 ;
: addr ( -- n ) r12 ;
: val ( -- n ) r11 ;
: temp ( -- n ) r10 ; 

{enum
: op-memory ( -- n ) literal ; enum,
: op-arithmetic ( -- n ) literal ; enum,
: op-shift ( -- n ) literal ; enum,
: op-logical ( -- n ) literal ; enum,
: op-compare ( -- n ) literal ; enum,
: op-branch ( -- n ) literal ; enum,
: op-move ( -- n ) literal ; enum,
: op-set ( -- n ) literal ; enum,
: op-swap ( -- n ) literal ; enum,
: op-misc ( -- n ) literal ; 
enum}

{enum
: 0m0000 ( -- n ) literal ; enum,
: 0m0001 ( -- n ) literal ; enum,
: 0m0010 ( -- n ) literal ; enum,
: 0m0011 ( -- n ) literal ; enum,
: 0m0100 ( -- n ) literal ; enum,
: 0m0101 ( -- n ) literal ; enum,
: 0m0110 ( -- n ) literal ; enum,
: 0m0111 ( -- n ) literal ; enum,
: 0m1000 ( -- n ) literal ; enum,
: 0m1001 ( -- n ) literal ; enum,
: 0m1010 ( -- n ) literal ; enum,
: 0m1011 ( -- n ) literal ; enum,
: 0m1100 ( -- n ) literal ; enum,
: 0m1101 ( -- n ) literal ; enum,
: 0m1110 ( -- n ) literal ; enum,
: 0m1111 ( -- n ) literal ;
enum}

: upper-half? ( value -- f ) 0m1100 bitwise-andu 0<> ;
: lower-half? ( value -- f ) 0m0011 bitwise-andu 0<> ; 







( output formatting )
: mask-imm32 ( index -- encoded ) 0xFFFFFFFF bitwise-andu ;
: mask-imm16 ( index -- encoded ) 0xFFFF bitwise-andu ;
: mask-imm8 ( index -- encoded ) 0xFF bitwise-andu ;
: mask-imm5 ( index -- encoded ) 0x1F bitwise-andu ;
: mask-imm4 ( index -- encoded ) 0xF bitwise-andu ;
: mask-imm3 ( index -- encoded ) 0x7 bitwise-andu ;
: mask-imm2 ( index -- encoded ) 0x3 bitwise-andu ;
: mask-flag ( index -- encoded ) 0x1 bitwise-andu ;
: to-position ( value starting-pos -- shifted ) <<u ;
: imm4-to-position ( value start -- shifted ) swap mask-imm4 swap to-position ;
: to-highest4 ( value -- encoded ) 12 imm4-to-position ;
: to-higher4 ( value -- encoded ) 8 imm4-to-position ;
: to-lower4 ( value -- encoded ) 4 imm4-to-position ;
: to-lowest4 ( value -- encoded ) mask-imm4 ;
: start-at-pos5 ( value -- encoded ) 5 to-position ;
: start-at-pos7 ( value -- encoded ) 7 to-position ;
: style3 ( index -- encoded ) mask-imm3 start-at-pos5 ;
: style2 ( index -- encoded ) mask-imm2 start-at-pos5 ;
: style4 ( index -- encoded ) mask-imm4 4 to-position ;
: set-flag ( flag position -- encoded ) swap mask-flag swap to-position ;
: set-bit-pos4 ( flag -- encoded ) 4 set-flag ;
: set-bit-pos5 ( flag -- encoded ) 5 set-flag ;
: set-bit-pos6 ( flag -- encoded ) 6 set-flag ;
: immediate-bit ( flag -- encoded ) set-bit-pos4 ;
: direction-bit ( flag -- encoded ) set-bit-pos5 ;
: link-bit ( flag -- encoded ) set-bit-pos5 ;
: cond-bit ( flag -- encoded ) set-bit-pos6 ;
: immediate-form ( -- encoded ) true immediate-bit ;
: indirect-form ( -- encoded ) false immediate-bit ;
: left ( -- encoded ) true direction-bit ;
: right ( -- encoded ) false direction-bit ;
: link ( -- encoded ) true link-bit ;
: nolink ( -- encoded ) false link-bit ;
: conditional ( -- encoded ) true cond-bit ;
: unconditional ( -- encoded ) false cond-bit ;

: combine2 ( a b -- c ) bitwise-oru ;
: combine3 ( a b c -- d ) 
  combine2 ( a e )
  combine2 ( d ) ;

: combine4 ( a b c d -- e ) 
  combine3 ( a f )
  combine2 ( e ) ;
variable current-bitmask
0m0000 current-bitmask !
: set-destination ( index -- encoded ) to-highest4 ;
: set-source ( index -- encoded ) to-higher4 ;
: set-offset ( index -- encoded ) to-highest4 ;
: set-opcode ( index -- encoded ) to-lowest4 ;
: set-source-and-dest ( src dest -- encoded ) set-destination swap set-source combine2 ;
: word: ( -- n ) 0 ;
: word, ( a b -- c ) combine2 ;
: ->highest4 ( imm4 code -- encoded ) swap to-highest4 word, ;
: ->destination ( destination code -- encoded ) ->highest4 ;
: ->source ( destination code -- encoded ) swap set-source word, ;
: ->dest,src ( src destination code -- encoded ) ->destination ->source ;
: ->lower4 ( imm4 code -- encoded ) swap to-lower4 word, ;
: ->style2 ( imm2 code -- encoded ) swap style2 word, ;
: ->style3 ( imm3 code -- encoded ) swap style3 word, ;
: ->style4 ( imm4 code -- encoded ) ->lower4 ;
: ->inst ( opcode -- encoded ) word: swap set-opcode word, ;
: ->bitmask ( bitmask code -- encoded ) 
  swap 
  dup current-bitmask ! \ need to save the current-bitmask so we can use it if needed
  to-higher4 word, ;

{enum
: linker-capacity ( -- n ) literal ; enum,
: linker-register ( -- n ) literal ; enum,
: linker-memory ( -- n ) literal ; 
enum}
variable capacity 
0x1000000 capacity ! 
: .register ( value index -- ) 
  swap ( index value )
  linker-register bin<<q 
  bin<<h
  bin<<q ;
: .capacity ( capacity -- ) 
  dup capacity ! \ set the new capacity
  linker-capacity bin<<q 
  bin<<h \ out goes the capacity
  0 bin<<q \ dump zero to the value
  ;
: memory-mask ( -- n ) capacity @ 1- ;

variable current-address
: .org ( value -- ) mask-imm32 current-address ! ;
0 .org 

: .here ( -- address ) current-address @ ;

: ++addr ( -- ) current-address @ 1+ memory-mask bitwise-andu current-address ! ;
: ->done ( instruction -- masked ) mask-imm16 
  linker-memory bin<<q
  \ output to our binary file at this point
  current-address @ bin<<h \ output the address
  ++addr 
  bin<<q ;
: .word16 ( value -- ) mask-imm16 ->done ;
: .word32 ( value -- ) 
  dup ( upper lower -- )
  .word16 \ put the lower into memory as is! 
  16 swap >>u .word16 \ followed by the upper 16
  ; 
: .word64 ( value -- )
  dup ( upper lower -- )
  .word32 \ put the lower 32 into memory as is!
  32 swap >>u .word32 \ followed by the upper 32
  ;

: deflabel ( -- ) variable ;
: .label ( -- variable ) variable$ ;
: is-here ( variable -- ) .here swap ! ;


: !move ( src dest bitmask -- ) 
  op-move ->inst ( src dest bitmask inst )
  ->lower4 ( src dest inst )
  ->dest,src ( inst )
  ->done ;


: !move8 ( src dest -- ) 0m0001 !move ;
: !move16 ( src dest -- ) 0m0011 !move ;
: !move24 ( src dest -- ) 0m0111 !move ;
: !move-upper16 ( src dest -- ) 0m1100 !move ;
: !move-lower16 ( src dest -- ) !move16 ;
: !move32 ( src dest -- ) 0m1111 !move ;
: !move0 ( src dest -- ) 0m0000 !move ;

: !swap ( src dest -- ) 
  op-swap ->inst
  ->dest,src
  ->done ; 

: !nop ( -- ) r0 r0 !swap ;

{enum 
: style-return ( -- n ) literal ; enum,
: style-terminate ( -- n ) literal ; enum,
: style-putc ( -- n ) literal ; enum,
: style-getc ( -- n ) literal ; 
enum}
: !misc ( body style -- ) 
  op-misc ->inst ( body style op )
  ->lower4 ( body op )
  word, ( op )
  ->done ;
: !ret ( -- ) 0 style-return !misc ;
: !terminate ( -- ) 0 style-terminate !misc ;
: !getc ( dest -- ) 
  0 ->destination 
  style-getc !misc ;
: !putc ( dest -- )
  0 ->destination
  style-putc !misc ;

{enum
: style-load ( -- n ) literal ; enum,
: style-store ( -- n ) literal ; enum,
: style-push ( -- n ) literal ; enum,
: style-pop ( -- n ) literal ; 
enum}

: !memory ( dest/offset bitmask kind2 -- ) 
  op-memory ->inst
  ->style2 
  ->bitmask
  ->highest4 \ could be a destination register or integer offset
  ->done ;

: !load ( offset bitmask -- ) style-load !memory ;
: !load8 ( offset -- ) 0m0001 !load ;
: !load8up ( offset -- ) 0m0010 !load ;
: !load16 ( offset -- ) 0m0011 !load ;
: !load16up ( offset -- ) 0m1100 !load ;
: !load32 ( offset -- ) 0m1111 !load ;
: !store ( offset bitmask -- ) style-store !memory ;
: !store8 ( offset -- ) 0m0001 !store ;
: !store8up ( offset -- ) 0m0010 !store ;
: !store16 ( offset -- ) 0m0011 !store ;
: !store16up ( offset -- ) 0m1100 !store ;
: !store32 ( offset -- ) 0m1111 !store ;
: !push ( dest bitmask -- ) style-push !memory ;
: !push16 ( dest -- ) 0m0011 !push ;
: !push16up ( dest -- ) 0m1100 !push ;
: !push32 ( dest -- ) 0m1111 !push ;
: !pop ( dest bitmask -- ) style-pop !memory ;
: !pop16 ( dest -- ) 0m0011 !pop ;
: !pop16up ( dest -- ) 0m1100 !pop ;
: !pop32 ( dest -- ) 0m1111 !pop ;
: !drop ( -- ) temp !pop32 ;
: !2drop ( -- ) !drop !drop ;


\ the instructions that have an immediate bit set are handled differently
\ the actual contents that differ between the two types are merged separately
\ with the common piece performed here
: ->shift ( body imm? -- )
  op-shift ->inst ( body imm? op )
  word, ( body op )
  word, ( op ) 
  ->done ;

: !shift ( src dest direction -- ) 
  ->dest,src 
  indirect-form word, ->shift ; 

: !shift-immediate ( imm5 dest direction -- )
  \ this has a different form
  ->destination ( imm5 op ) 
  swap ( op imm5 )
  mask-imm5 ( op imm5m )
  start-at-pos7 ( op imm5m7s )
  word, ( op )
  immediate-form word, ( op )
  ->shift ;

: !shift-left ( src dest -- ) left !shift ;
: !shift-right ( src dest -- ) right !shift ;
: !shift-left-immediate ( imm5 dest -- ) left !shift-immediate ;
: !shift-right-immediate ( imm5 dest -- ) right !shift-immediate ;

: !2* ( dest -- ) 1 swap !shift-left-immediate ;
: !2/ ( dest -- ) 1 swap !shift-right-immediate ;
: !4* ( dest -- ) 2 swap !shift-left-immediate ;
: !4/ ( dest -- ) 2 swap !shift-right-immediate ;
: !8* ( dest -- ) 3 swap !shift-left-immediate ;
: !8/ ( dest -- ) 3 swap !shift-right-immediate ;
: !16* ( dest -- ) 4 swap !shift-left-immediate ;
: !16/ ( dest -- ) 4 swap !shift-right-immediate ;

: ->branch ( body imm? -- ) 
  op-branch ->inst ( b i op )
  word, ( b op )
  word, ( op ) 
  ->done ;

: !branch-indirect ( dest cond? link? -- ) 
  word, ( d b )
  ->destination ( body )
  indirect-form ( body imm ) ->branch ;
: !branch ( address cond? link? -- ) 
  word, ( a b )
  immediate-form ->branch ( address )
  .word32 \ we are basically putting a .word32 after the fact
  ;

: !call ( address cond? -- ) link !branch ;
: !jump ( address cond? -- ) nolink !branch ;
: !call-indirect ( dest cond? -- ) link !branch-indirect ;
: !jump-indirect ( dest cond? -- ) nolink !branch-indirect ;

\ flag order does not matter but provide macros
: !jump-unconditional-indirect ( dest -- ) unconditional !jump-indirect ;
: !jump-conditional-indirect ( dest -- ) conditional !jump-indirect;
: !jump-unconditional ( imm -- ) unconditional !jump ;
: !jump-conditional ( imm -- ) conditional !jump ;
: !call-unconditional-indirect ( dest -- ) unconditional !call-indirect ;
: !call-unconditional ( imm -- ) unconditional !call ;
: !call-conditional-indirect ( dest -- ) conditional !call-indirect ;
: !call-conditional ( imm -- ) conditional !call ;

: !jui ( dest -- ) !jump-unconditional-indirect ;
: !ju ( imm -- ) !jump-unconditional ;
: !jci ( dest -- ) !jump-conditional-indirect ;
: !jc ( imm -- ) !jump-conditional ;
: !cui ( dest -- ) !call-unconditional-indirect ;
: !cu ( imm -- ) !call-unconditional ;
: !cci ( dest -- ) !call-conditional-indirect ;
: !cc ( imm -- ) !call-conditional ;

: ->set ( dest mask -- ) 
  op-set ->inst 
  ->bitmask
  ->destination 
  ->done ;
: get-upper-half ( value -- upper ) 16 >>u mask-imm16 ;
: get-lower-half ( value -- lower ) mask-imm16 ;
: split-into-halves ( value -- upper lower ) 
  dup ( value value )
  get-upper-half ( value upper )
  swap ( upper value )
  get-lower-half ( upper lower )
  ;
: emit-data16-on-true ( value cond -- ) if .data16 else drop then ;
: emit-immediate ( immediate -- ) 
  split-into-halves
  current-bitmask @ lower-half? emit-data16-on-true 
  current-bitmask @ upper-half? emit-data16-on-true ;

: !set ( imm dest mask -- )
  ->set \ setup the initial word 
  emit-immediate ;
: !set32 ( imm dest -- ) 0m1111 !set ;
: !set16 ( imm dest -- ) 0m0011 !set ;
: !set16u ( imm dest -- ) 0m1100 !set ;
: !set8 ( imm dest -- ) 0m0001 !set ;
: !set0 ( dest -- ) 0 swap 0m0000 !set ;

{enum
: style-add ( -- n ) literal ; enum,
: style-sub ( -- n ) literal ; enum,
: style-mul ( -- n ) literal ; enum,
: style-div ( -- n ) literal ; enum,
: style-rem ( -- n ) literal ; enum,
: style-min ( -- n ) literal ; enum,
: style-max ( -- n ) literal ; 
enum}

: ->arithmetic ( body imm? -- )
  op-arithmetic ->inst ( body imm? op )
  word, ( body op ) 
  word, ( op )
  ->done ;

: !arithmetic ( src dest style -- )
  0 ->style3 ( src dest body )
  ->dest,src ( body )
  indirect-form ->arithmetic ;

: !arithmetic-immediate ( immediate dest bitmask style -- )
  0 ->style3 ( immediate dest bitmask body )
  ->bitmask ( immediate dest body )
  ->destination ( immediate body )
  immediate-form ->arithmetic \ output the first word
  emit-immediate \ then try and see if there are other words to output following it
  ;
: !add ( src dest -- ) style-add !arithmetic ;
: !addi ( immediate dest bitmask -- ) style-add !arithmetic-immediate ;
: !addi32 ( immediate dest -- ) 0m1111 !addi ;
: !addi24 ( immediate dest -- ) 0m0111 !addi ;
: !addi16 ( immediate dest -- ) 0m0011 !addi ;
: !addi8  ( immediate dest -- ) 0m0001 !addi ;

: !sub ( src dest -- ) style-sub !arithmetic ;
: !subi ( immediate dest bitmask -- ) style-sub !arithmetic-immediate ;
: !subi32 ( immediate dest -- ) 0m1111 !subi ;
: !subi24 ( immediate dest -- ) 0m0111 !subi ;
: !subi16 ( immediate dest -- ) 0m0011 !subi ;
: !subi8  ( immediate dest -- ) 0m0001 !subi ;

: !mul ( src dest -- ) style-mul !arithmetic ;
: !muli ( immediate dest bitmask -- ) style-mul !arithmetic-immediate ;
: !muli32 ( immediate dest -- ) 0m1111 !muli ;
: !muli24 ( immediate dest -- ) 0m0111 !muli ;
: !muli16 ( immediate dest -- ) 0m0011 !muli ;
: !muli8  ( immediate dest -- ) 0m0001 !muli ;

: !div ( src dest -- ) style-div !arithmetic ;
: !divi ( immediate dest bitmask -- ) style-div !arithmetic-immediate ;
: !divi32 ( immediate dest -- ) 0m1111 !divi ;
: !divi24 ( immediate dest -- ) 0m0111 !divi ;
: !divi16 ( immediate dest -- ) 0m0011 !divi ;
: !divi8  ( immediate dest -- ) 0m0001 !divi ;

: !rem ( src dest -- ) style-rem !arithmetic ;
: !remi ( immediate dest bitmask -- ) style-rem !arithmetic-immediate ;
: !remi32 ( immediate dest -- ) 0m1111 !remi ;
: !remi24 ( immediate dest -- ) 0m0111 !remi ;
: !remi16 ( immediate dest -- ) 0m0011 !remi ;
: !remi8  ( immediate dest -- ) 0m0001 !remi ;

: !min ( src dest -- ) style-min !arithmetic ;
: !mini ( immediate dest bitmask -- ) style-min !arithmetic-immediate ;
: !mini32 ( immediate dest -- ) 0m1111 !mini ;
: !mini24 ( immediate dest -- ) 0m0111 !mini ;
: !mini16 ( immediate dest -- ) 0m0011 !mini ;
: !mini8  ( immediate dest -- ) 0m0001 !mini ;

: !max ( src dest -- ) style-max !arithmetic ;
: !maxi ( immediate dest bitmask -- ) style-max !arithmetic-immediate ;
: !maxi32 ( immediate dest -- ) 0m1111 !maxi ;
: !maxi24 ( immediate dest -- ) 0m0111 !maxi ;
: !maxi16 ( immediate dest -- ) 0m0011 !maxi ;
: !maxi8  ( immediate dest -- ) 0m0001 !maxi ;

: !1+ ( dest -- ) 1 swap !addi8 ;
: !1- ( dest -- ) 1 swap !subi8 ;
: !2+ ( dest -- ) 2 swap !addi8 ;
: !2- ( dest -- ) 2 swap !addi8 ;
: !4+ ( dest -- ) 4 swap !addi8 ;
: !4- ( dest -- ) 4 swap !addi8 ;
: !8+ ( dest -- ) 8 swap !addi8 ;
: !8- ( dest -- ) 8 swap !addi8 ;

: !three-operand-form ( src1 dest -- dest )
  tuck ( src2 dest src1 dest )
  !move32 \ transfer src1 to the destination
  ;
: !add3 ( src2 src1 dest -- )
  !three-operand-form 
  !add ;

: !sub3 ( src2 src1 dest -- )
  !three-operand-form 
  !sub ;

: !mul3 ( src2 src1 dest -- ) 
  !three-operand-form 
  !mul ;

: !div3 ( src2 src1 dest -- ) 
  !three-operand-form 
  !div ;

: !rem3 ( src2 src1 dest -- ) 
  !three-operand-form 
  !rem ;

: !max3 ( src2 src1 dest -- ) 
  !three-operand-form 
  !max ;

: !min3 ( src2 src1 dest -- ) 
  !three-operand-form 
  !min ;

{enum
: style-and ( -- n ) literal ; enum,
: style-or ( -- n ) literal ; enum,
: style-xor ( -- n ) literal ; enum,
: style-nand ( -- n ) literal ; enum,
: style-not ( -- n ) literal ; 
enum}

: ->logical ( body imm? -- ) 
  op-logical ->inst ( body imm? op )
  word, ( body op )
  word, ( op )
  ->done ;

: !logical ( src dest style -- )
  0 ->style3
  ->dest,src ( body )
  indirect-form ->logical ;

: !logical-immediate ( immediate dest bitmask style -- )
  0 ->style3
  ->bitmask
  ->destination
  immediate-form ->logical \ first word
  emit-immediate ;

: !and ( src dest -- ) style-and !logical ;
: !andi ( immediate dest bitmask -- ) style-and !logical-immediate ;
: !andi32 ( immediate dest bitmask -- ) 0m1111 !andi ;
: !andi24 ( immediate dest bitmask -- ) 0m0111 !andi ;
: !andi16 ( immediate dest bitmask -- ) 0m0011 !andi ;
: !andi8  ( immediate dest bitmask -- ) 0m0001 !andi ;

: !or ( src dest -- ) style-or !logical ;
: !ori ( immediate dest bitmask -- ) style-or !logical-immediate ;
: !ori32 ( immediate dest bitmask -- ) 0m1111 !ori ;
: !ori24 ( immediate dest bitmask -- ) 0m0111 !ori ;
: !ori16 ( immediate dest bitmask -- ) 0m0011 !ori ;
: !ori8  ( immediate dest bitmask -- ) 0m0001 !ori ;

: !xor ( src dest -- ) style-xor !logical ;
: !xori ( immediate dest bitmask -- ) style-xor !logical-immediate ;
: !xori32 ( immediate dest bitmask -- ) 0m1111 !xori ;
: !xori24 ( immediate dest bitmask -- ) 0m0111 !xori ;
: !xori16 ( immediate dest bitmask -- ) 0m0011 !xori ;
: !xori8  ( immediate dest bitmask -- ) 0m0001 !xori ;

: !nand ( src dest -- ) style-nand !logical ;
: !nandi ( immediate dest bitmask -- ) style-nand !logical-immediate ;
: !nandi32 ( immediate dest bitmask -- ) 0m1111 !nandi ;
: !nandi24 ( immediate dest bitmask -- ) 0m0111 !nandi ;
: !nandi16 ( immediate dest bitmask -- ) 0m0011 !nandi ;
: !nandi8  ( immediate dest bitmask -- ) 0m0001 !nandi ;

: !not ( src dest -- ) style-not !logical ;
: !noti ( immediate dest bitmask -- ) style-not !logical-immediate ;
: !noti32 ( immediate dest bitmask -- ) 0m1111 !noti ;
: !noti24 ( immediate dest bitmask -- ) 0m0111 !noti ;
: !noti16 ( immediate dest bitmask -- ) 0m0011 !noti ;
: !noti8  ( immediate dest bitmask -- ) 0m0001 !noti ;

: !putci ( immediate -- ) 
  temp tuck !set8 
  !putc ;

: !cr ( -- ) 0xA !putci ;
: !space ( -- ) 0x20 !putci ;

{enum
: style-equals ( -- n ) literal ; enum,
: style-not-equals ( -- n ) literal ; enum,
: style-less-than ( -- n ) literal ; enum,
: style-greater-than ( -- n ) literal ; enum,
: style-less-than-or-equal-to ( -- n ) literal ; enum,
: style-greater-than-or-equal-to ( -- n ) literal ; enum,
: style-move-from-condition ( -- n ) literal ; enum,
: style-move-to-condition ( -- n ) literal ;
enum}

: ->compare ( body imm? -- )
  op-compare ->inst ( body imm? op )
  word, ( body op )
  word, 
  ->done ;
: !single-reg-compare ( dest style -- )
  0 ->style3 
  ->destination
  indirect-form ->compare ;
: !compare ( src dest style -- )
  0 ->style3 
  ->dest,src 
  indirect-form ->compare ;
: !compare-immediate ( immediate dest bitmask style -- )
  0 ->style3
  ->bitmask
  ->destination
  immediate-form ->compare 
  emit-immediate ;
: !eq ( src dest -- ) style-equals !compare ;
: !eqi ( immediate dest bitmask -- ) style-equals !compare-immediate ;
: !eqi32 ( immediate dest -- ) 0m1111 !eqi ;
: !eqi24 ( immediate dest -- ) 0m0111 !eqi ;
: !eqi16 ( immediate dest -- ) 0m0011 !eqi ;
: !eqi8  ( immediate dest -- ) 0m0001 !eqi ;

: !neq ( src dest -- ) style-not-equals !compare ;
: !neqi ( immediate dest bitmask -- ) style-not-equals !compare-immediate ;
: !neqi32 ( immediate dest -- ) 0m1111 !neqi ;
: !neqi24 ( immediate dest -- ) 0m0111 !neqi ;
: !neqi16 ( immediate dest -- ) 0m0011 !neqi ;
: !neqi8  ( immediate dest -- ) 0m0001 !neqi ;

: !lt ( src dest -- ) style-less-than !compare ;
: !lti ( immediate dest bitmask -- ) style-less-than !compare-immediate ;
: !lti32 ( immediate dest -- ) 0m1111 !lti ;
: !lti24 ( immediate dest -- ) 0m0111 !lti ;
: !lti16 ( immediate dest -- ) 0m0011 !lti ;
: !lti8  ( immediate dest -- ) 0m0001 !lti ;

: !gt ( src dest -- ) style-greater-than !compare ;
: !gti ( immediate dest bitmask -- ) style-greater-than !compare-immediate ;
: !gti32 ( immediate dest -- ) 0m1111 !gti ;
: !gti24 ( immediate dest -- ) 0m0111 !gti ;
: !gti16 ( immediate dest -- ) 0m0011 !gti ;
: !gti8  ( immediate dest -- ) 0m0001 !gti ;

: !le ( src dest -- ) style-less-than-or-equal-to !compare ;
: !lei ( immediate dest bitmask -- ) style-less-than-or-equal-to !compare-immediate ;
: !lei32 ( immediate dest -- ) 0m1111 !lei ;
: !lei24 ( immediate dest -- ) 0m0111 !lei ;
: !lei16 ( immediate dest -- ) 0m0011 !lei ;
: !lei8  ( immediate dest -- ) 0m0001 !lei ;

: !ge ( src dest -- ) style-greater-than-or-equal-to !compare ;
: !gei ( immediate dest bitmask -- ) style-greater-than-or-equal-to !compare-immediate ;
: !gei32 ( immediate dest -- ) 0m1111 !gei ;
: !gei24 ( immediate dest -- ) 0m0111 !gei ;
: !gei16 ( immediate dest -- ) 0m0011 !gei ;
: !gei8  ( immediate dest -- ) 0m0001 !gei ;

: 0swap ( a -- 0 a ) 0 swap ;
: 0swap0m ( a -- 0 a 0m0000 ) 0swap 0m0000 ;
: !eqz    ( dest -- ) 0swap0m !eqi ;
: !neqz   ( dest -- ) 0swap0m !neqi ;
: !ltz    ( dest -- ) 0swap0m !lti ; 
: !gtz    ( dest -- ) 0swap0m !gti ;
: !lez    ( dest -- ) 0swap0m !lei ;
: !gez    ( dest -- ) 0swap0m !gei ;



: !reg<-c  ( dest -- ) style-move-from-condition !single-reg-compare ;
: !reg->c ( dest -- ) style-move-to-condition !single-reg-compare ;

: !true->c ( -- ) true temp tuck !mov8 !reg->c ;
: !false->c ( -- ) false temp tuck !mov0 !reg->c ;
: !value->addr ( -- ) value addr !move32 ;
: !addr->value ( -- ) addr value !move32 ;
: !sp<->csp ( -- ) sp csp !swap ;
: tempdest ( a -- temp a ) temp swap ;
: !push-immediate ( imm bitmask -- ) 
  dup ( imm bitmask bitmask )
  -rot ( bitmask imm bitmask )
  tempdest ( bitmask imm temp bitmask )
  !set ( bitmask )
  tempdest ( temp bitmask ) 
  !push ;

: !push-immediate32 ( imm -- ) 0m1111 !push-immediate ;
: !push-immediate24 ( imm -- ) 0m0111 !push-immediate ;
: !push-immediate16 ( imm -- ) 0m0011 !push-immediate ;
: !push-immediate8  ( imm -- ) 0m0001 !push-immediate ;

: !pop-subroutine ( dest bitmask -- ) !sp<->csp !pop !sp<->csp ;
: !pop-subroutine32 ( immediate -- ) 0m1111 !pop-subroutine ;
: !pop-subroutine24 ( immediate -- ) 0m0111 !pop-subroutine ;
: !pop-subroutine16 ( immediate -- ) 0m0011 !pop-subroutine ;
: !pop-subroutine8  ( immediate -- ) 0m0001 !pop-subroutine ;

: !push-subroutine ( dest bitmask -- ) !sp<->csp !push !sp<->csp ;
: !push-immediate-subroutine ( immediate bitmask -- ) !sp<->csp !push-immediate !sp<->!csp ;
: !push-immediate-subroutine32 ( immediate -- ) 0m1111 !push-immediate-subroutine ;
: !push-immediate-subroutine24 ( immediate -- ) 0m0111 !push-immediate-subroutine ;
: !push-immediate-subroutine16 ( immediate -- ) 0m0011 !push-immediate-subroutine ;
: !push-immediate-subroutine8  ( immediate -- ) 0m0001 !push-immediate-subroutine ;

: !param->subroutine ( reg bitmask -- ) 
  \ move a value from the top of the parameter stack to the top of the subroutine stack
  2dup ( reg bitmask reg bitmask )
  !pop 
  !push-subroutine ;

: !param<-subroutine ( reg bitmask -- )
  2dup ( reg bitmask reg bitmask )
  !pop-subroutine
  !push ;

: !tos ( reg bitmask -- reg ) \ load the top of the stack and use that in the next computation
  over ( reg bitmask reg ) 
  swap ( reg reg bitmask )
  !pop ;



close-input-file

