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
    push    0         # r15
    push    0         # r14
    push    0         # r13
    push    0         # r12
    push    0         # r11
    push    0         # r10
    push    0         # r9
    push    0         # r8
    push    rdx       # rdi
    push    0         # rsi
    push    0         # rdx
    push    0         # rcx
    push    0         # rbx
    push    0         # rax
    mov     rax,rsp   # set current rsp to return value
    mov     rsp,rcx   # load rsp from rcx
    sti
    ret

.global     switch_context
.align      16
switch_context:   # void switch_context(uint64_t *current_rsp, uint64_t next_rsp);
# push current task's general registers to stack
    cli
    push    rbp
    push    r15
    push    r14
    push    r13
    push    r12
    push    r11
    push    r10
    push    r9
    push    r8
    push    rdi
    push    rsi
    push    rdx
    push    rcx
    push    rbx
    push    rax
# switch rsp
    mov     [rdi],rsp
    mov     rsp,rsi
# pop next task's general registers from stack
    pop     rax
    pop     rbx
    pop     rcx
    pop     rdx
    pop     rsi
    pop     rdi
    pop     r8
    pop     r9
    pop     r10
    pop     r11
    pop     r12
    pop     r13
    pop     r14
    pop     r15
    pop     rbp
# return to next task
    sti
    ret

.global     switch_fork
.align      16
switch_fork:    # void switch_fork(uint64_t **cur_rsp, uint64_t *new_stack_top);
# push current task's general registers to stack
    cli
    #hlt
    pop     rdx // pop instruction pointer
    mov     [rsi-0],rdx // push to new_stack
    push    rdx // push instruction pointer
    mov     [rsi-8],rbp // push rbp to new_stack
    push    rbp
    mov     [rsi-16],r15 // push r15 to newstack
    push    r15
    mov     [rsi-24],r14
    push    r14
    mov     [rsi-32],r13
    push    r13
    mov     [rsi-40],r12
    push    r12
    mov     [rsi-48],r11
    push    r11
    mov     [rsi-56],r10
    push    r10
    mov     [rsi-64],r9
    push    r9
    mov     [rsi-72],r8
    push    r8
    mov     [rsi-80],rdi
    push    rdi
    mov     [rsi-88],rsi
    push    rsi
    mov     [rsi-96],rdx
    push    rdx
    mov     [rsi-104],rcx
    push    rcx
    mov     [rsi-112],rbx
    push    rbx
    mov     [rsi-120],rax
    push    rax
# switch rsp
    mov     [rdi],rsp
    sub     rsi,128
    mov     rsp,rsi
    #hlt
# pop next task's general registers from stack
    pop     rax
    pop     rbx
    pop     rcx
    pop     rdx
    pop     rsi
    pop     rdi
    pop     r8
    pop     r9
    pop     r10
    pop     r11
    pop     r12
    pop     r13
    pop     r14
    pop     r15
    pop     rbp
# return to next task
    #hlt
    sti
    ret

