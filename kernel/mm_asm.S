.intel_syntax   noprefix
.code64

.global     load_gdt    # void load_gdt(uint64_t base, int limit);
load_gdt:
    push    rbp
    mov     rbp,rsp
    sub     rsp,0x10
    mov     rdx,rsp
    mov     [rdx],si
    mov     [rdx + 2],rdi
    lgdt    [rdx]
    add     rsp,0x10
    mov     rsp,rbp
    pop     rbp
    ret

.global     intersegment_jump   # void intersegment_jump(uint16_t cs);
intersegment_jump:
    push    rbp
    mov     rbp,rsp
    cli
    xor     cx,cx
    mov     ds,cx
    mov     es,cx
    mov     ss,cx
    mov     gs,cx
    mov     fs,cx

    lea     rcx,label
    push    rdi     # cs
    push    rcx     # rip
    rex64 retf
label:
    sti
    mov     rsp,rbp
    pop     rbp
    ret

.global     load_pgtable    # void load_pgtable(uint64_t *pgtable);
load_pgtable:
    mov     cr3,rdi
    ret

