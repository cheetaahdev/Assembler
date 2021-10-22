.equ lit, 229
.global tim_cfg, isr
.section match
.word  12, 87
.skip 4
.global rec
.extern skok
push r1
ldr r1, r2
push psw 
jmp rec
.word skok
rec: ldr sp, [r3 + 17]
jmp *022
jmp lit
jne *lit
jgt *r1
jeq *[r2]
jne *[r3 + 17]
jmp *[r4 + 7]
ldr r1, lit
ldr pc, $lit
ldr r3, $2
not psw 
xor r5, r4
div r1, r2
shl r5, r3
cmp r4, pc
pop psw
pop r5
