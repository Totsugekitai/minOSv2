.intel_syntax   noprefix
.code64

.global     io_in8
io_in8:
    mov     rdx,rdi
    in      al,dx
    ret

.global     io_out8
io_out8:
    mov     rdx,rdi
    mov     rax,rsi
    out     dx,al
    ret

.global     io_in32
io_in32:
    mov     rdx,rdi
    in      eax,dx
    ret

.global     io_out32
io_out32:
    mov     rdx,rdi
    mov     rax,rsi
    out     dx,eax
    ret

