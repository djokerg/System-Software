.extern my_counter
.global handler
.section my_code_handler
handler:
push %r1
push %r2
csrrd %cause, %r1
ld $2, %r2
beq %r1, %r2, my_isr_timer
ld $3, %r2
beq %r1, %r2, my_isr_terminal
# obrada prekida od tajmera
my_isr_timer:
jmp finish
# obrada prekida od terminala
my_isr_terminal:
ld my_counter, %r1
ld $1, %r2
add %r2, %r1
st %r1, my_counter
finish:
pop %r2
pop %r1
iret
.section my_code_main
wait: .word 0
.ascii "Marmun"
.end