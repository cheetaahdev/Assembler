.section text
.equ apsolutni, 0x756

ldr r1, $5
a: ldr r2, $067
ldr r3, $0xff45

ldr r3, a
ldr r4, b 

.section data
b: push r0
ldr r6, 45

ldr r3, $a
ldr r0, r4 
ldr r5, r6 
ldr r5, [r6]
ldr r5, [r5 + 0xf]
ldr r5, [r5 + a]
ldr r5, apsolutni
ldr r5, $apsolutni

jmp  %a #originalno bilo ldr psw, u ovoj i narednoj
jmp  %apsolutni

.end
