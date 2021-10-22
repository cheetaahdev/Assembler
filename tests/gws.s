# provera za globalne simbole i .word direktivu

.global a, b, c 
.extern eksterni
.section text
.word firstSymbol

firstSymbol: 
    push r0
a: push r1 
b: push r2 
c: push r3
d: push r5
k: push r6
.word 433
.word 0x45
.word 045
.word a 
.word a, b, c, d, 0x45
.skip 4
.skip 0x7
.skip 04
.word eksterni

.end
