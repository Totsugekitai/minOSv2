.intel_syntax   noprefix
.code64

.global     set_idt     # void set_idt(uint64_t idt_addr, uint16_t size);
set_idt:
    sub     rsp,0x10
    mov     [rsp],si
    mov     [rsp+2],rdi
    lidt    [rsp]
    add     rsp,0x10
    ret

