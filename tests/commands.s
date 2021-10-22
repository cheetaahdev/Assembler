# asemblerske naredbe test 1 

.section text
halt
int r5
iret 
ret 
push pc 
pop psw 
xchg r0, pc 
add r0, r0
sub r4, r5 
mul pc, sp 
div pc, psw
cmp r4, r6 
not r6 
and r5, r5 
or r7, r3 
xor r2,    r5 
test r4, r6 
shl r4, r2 
shr r4, r2 
.end
