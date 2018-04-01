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

{enum
: style-add ( -- n ) literal ; enum,
: style-sub ( -- n ) literal ; enum,
: style-mul ( -- n ) literal ; enum,
: style-div ( -- n ) literal ; enum,
: style-rem ( -- n ) literal ; enum,
: style-min ( -- n ) literal ; enum,
: style-max ( -- n ) literal ; 
enum}

{enum
: style-and ( -- n ) literal ; enum,
: style-or ( -- n ) literal ; enum,
: style-xor ( -- n ) literal ; enum,
: style-nand ( -- n ) literal ; enum,
: style-not ( -- n ) literal ; 
enum}

{enum
: style-load ( -- n ) literal ; enum,
: style-store ( -- n ) literal ; enum,
: style-push ( -- n ) literal ; enum,
: style-pop ( -- n ) literal ; 
enum}

{enum 
: style-return ( -- n ) literal ; enum,
: style-terminate ( -- n ) literal ; 
enum}

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
: ->bitmask ( bitmask code -- encoded ) swap to-higher4 word, ;

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

: !misc ( style -- ) op-misc ->inst ->lower4 ->done ;
: !ret ( -- ) style-return !misc ;
: !terminate ( -- ) style-terminate !misc ;

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
: !double ( dest -- ) !2* ;
: !2/ ( dest -- ) 1 swap !shift-right-immediate ;
: !halve ( dest -- ) !2/ ;
: !4* ( dest -- ) 2 swap !shift-left-immediate ;
: !4/ ( dest -- ) 2 swap !shift-right-immediate ;
: !quarter ( dest -- ) !4/ ;
: !8* ( dest -- ) 3 swap !shift-left-immediate ;
: !8/ ( dest -- ) 3 swap !shift-right-immediate ;
: !eighth ( dest -- ) !8/ ;
: !16* ( dest -- ) 4 swap !shift-left-immediate ;
: !16/ ( dest -- ) 4 swap !shift-right-immediate ;
: !sixteenth ( dest -- ) !16/ ;

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

\ flag order does not matter but provide macros
: !branch-unconditional-indirect ( dest -- ) unconditional nolink !branch-indirect ;
: !branch-unconditional ( imm -- ) unconditional nolink !branch ;
: !branch-conditional-indirect ( dest -- ) conditional nolink !branch-indirect ;
: !branch-conditional ( imm -- ) conditional nolink !branch ;
: !call-unconditional-indirect ( dest -- ) unconditional link !branch-indirect ;
: !call-unconditional ( imm -- ) unconditional link !branch ;
: !call-conditional-indirect ( dest -- ) conditional link !branch-indirect ;
: !call-conditional ( imm -- ) conditional link !branch ;

: !bui ( dest -- ) !branch-unconditional-indirect ;
: !bu ( imm -- ) !branch-unconditional ;
: !bci ( dest -- ) !branch-conditional-indirect ;
: !bc ( imm -- ) !branch-conditional ;
: !cui ( dest -- ) !call-unconditional-indirect ;
: !cu ( imm -- ) !call-unconditional ;
: !cci ( dest -- ) !call-conditional-indirect ;
: !cc ( imm -- ) !call-conditional ;

\ set is a little strange since we have to be able to decompose the instruction
\ into multiple 16-bit words based on the bitmask

close-input-file

