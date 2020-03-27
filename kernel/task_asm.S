.intel_syntax   noprefix

.global     init_stack
.align      16
init_stack:           # uint64_t *init_stack(uint64_t stack_bottom, uint64_t rip, struct thread *thread);
    cli
    mov     rcx,rsp   # save rsp at rcx
    mov     rsp,rdi   # move rip to rsp
# push general registers to stack
    push    rsi       # push rip(second argument) to stack bottom
                      # because function switch_context requires return rip
    push    0         # rbp
    push    0         # r11
    push    0         # r10
    push    0         # r9
    push    0         # r8
    push    rdx       # rdi
    push    0         # rsi
    push    0         # rcx
    push    0         # rdx
    push    0         # rax
    mov     rax,rsp   # set current rsp to return value
    mov     rsp,rcx   # load rsp from rcx
    sti
    ret

.global     switch_context
.align      16
switch_context:   # void switch_context(uint64_t *current_rsp, uint64_t next_rsp);
# push current task's general registers to stack
    push    rbp
    push    r11
    push    r10
    push    r9
    push    r8
    push    rdi
    push    rsi
    push    rdx
    push    rcx
    push    rax
# switch rsp
    mov     [rdi],rsp
    mov     rsp,rsi
# pop next task's general registers from stack
    pop     rax
    pop     rcx
    pop     rdx
    pop     rsi
    pop     rdi
    pop     r8
    pop     r9
    pop     r10
    pop     r11
    pop     rbp
# return to next task
    ret

