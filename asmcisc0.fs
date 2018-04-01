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
: direction-left ( -- encoded ) true direction-bit ;
: direction-right ( -- encoded ) false direction-bit ;
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
: ->destination ( destination code -- encoded ) swap to-destination word, ;
: ->source ( destination code -- encoded ) swap to-source word, ;
: ->lower4 ( imm4 code -- encoded ) swap to-lower4 word, ;
: ->inst ( opcode -- encoded ) word: swap set-opcode word, ;
: ->done ( instruction -- masked ) mask-imm16 ;
: !move ( src dest bitmask -- encoded ) 
  op-move ->inst ( src dest bitmask inst )
  ->lower4 ( src dest inst )
  ->destination ( src inst )
  ->source ( inst ) 
  ->done ;


: !move8 ( src dest -- encoded ) 0m0001 !move ;
: !move16 ( src dest -- encoded ) 0m0011 !move ;
: !move24 ( src dest -- encoded ) 0m0111 !move ;
: !move-upper16 ( src dest -- encoded ) 0m1100 !move ;
: !move-lower16 ( src dest -- encoded ) !move16 ;
: !move32 ( src dest -- encoded ) 0m1111 !move ;
: !move0 ( src dest -- encoded ) 0m0000 !move ;

: !swap ( src dest -- encoded ) 
  op-swap ->inst
  ->destination ( src op )
  ->source ( inst esrc )
  ->done ; 

: !nop ( -- encoded ) r0 r0 !swap ;


close-input-file

