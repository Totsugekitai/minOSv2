
bits 64
section .text

global     load_gdt    ; void load_gdt(uint64_t base, int limit);
load_gdt:
    push    rbp
    mov     rbp,rsp
    mov     rdx,0x10
    mov     [rdx],si
    mov     [rdx + 2],rdi
    lgdt    [rdx]
    mov     rsp,rbp
    pop     rbp
    ret

global     intersegment_jump   ; void intersegment_jump(uint16_t cs);
intersegment_jump:
    push    rbp
    mov     rbp,rsp

    xor     cx,cx
    mov     ds,cx
    mov     es,cx
    mov     ss,cx
    mov     gs,cx
    mov     fs,cx

    lea     rcx,.label
    push    rdi     ; cs
    push    rcx     ; rip
    o64     retf
    ;mov     rcx,rsp
    ;push    0       ; ss = 0に設定
    ;push    rcx     ; rspはそのまま
    ;pushfq          ; RFLAGSはそのまま
    ;push    rdi     ; cs = rdi(第一引数)に設定(権限のみ見る)
    ;lea     rcx,label
    ;push    rcx  ; rip = main_labelのアドレス
    ;iretq           ; pushした値をそれぞれに書き込み、main_labelにジャンプ
.label:
    mov     rsp,rbp
    pop     rbp
    ret

