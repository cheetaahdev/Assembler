#asemblerske naredbe test 2 , skokovi

.section text 
.extern ekss
.equ apsolutni, 0x7f45

a: 
 call 54
 b: call 67
 call 0xf

 jmp 0x7f43
 jne 34
 jgt 10

 jgt *10
 jgt *0x7f65
 jmp b
 jmp *b
 jmp %b
 
 jmp *r4
 jmp *[r4]
 jmp *[r4 + 0x21]
 jmp *[r4 + b]

.section new 


jmp %eks2
.extern eks2
 push r4
jmp %apsolutni 
jmp apsolutni 

.end