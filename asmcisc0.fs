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
: opmemory ( -- n ) literal ; enum,
: oparithmetic ( -- n ) literal ; enum,
: opshift ( -- n ) literal ; enum,
: oplogical ( -- n ) literal ; enum,
: opcompare ( -- n ) literal ; enum,
: opbranch ( -- n ) literal ; enum,
: opmove ( -- n ) literal ; enum,
: opset ( -- n ) literal ; enum,
: opswap ( -- n ) literal ; enum,
: opmisc ( -- n ) literal ; 
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

: immediate ( -- n ) 1 ;
: imm4 ( index -- encoded ) 0xF bitwise-andu ;
: destination ( index -- encoded ) imm4 12 <<u ;
: source ( index -- encoded ) imm4 8 <<u ;
: offset ( index -- encoded ) imm4 12 <<u ;

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

close-input-file

