section .text

;--------------------------------------
; Entry: RDI = destination pointer
; Exit: RDI = input num
; Expects: R14 is the read buffer ptr
; Destroys: RAX, RBX, RCX, RDX, RSI, RDI
;--------------------------------------
_scanf:
            push r11
            push r12
            push r13
            push r8             ; save registers from syscall
            push rax
            push r14
            push rdi
            xor rax, rax        ; syscall read
            xor rdi, rdi        ; 0 is the stdin descriptor
            mov rsi, r14        ; syscall read
            mov rdx, 20         ; len of buffer
            syscall
            pop rdi
            pop r14
            pop rax
            pop r8              ; restore registers
            pop r13
            pop r12
            pop r11

            push r14
            push rbx            ; save registers destroyed by strlen
            push rax
            call strlen         ; len of input string
            xor rax, rax
            xor rdx, rdx
            xor r8, r8
            xor rcx, rcx
            call read_buf     ; from string to int

            mov [rdi], r8       ; store result in dest
            pop rax
            pop rbx
            pop r14
            ret


;--------------------------------------
; Entry: RBX = len of string
; Exit: RDI = input num
; Expects: R14 is the read buffer ptr
; Destroys: RAX, RCX, R8
;--------------------------------------

strlen:     xor rbx, rbx        ; len will be in rbx
            push r14            ; save read buffer

.next:      mov eax, [r14]      ; first symbol in eax
            mov dl, al
            xor eax, eax        ; first symbol in al
            mov al, dl
            sub eax, '0'
            cmp eax, 0          ; check if not a number
            jl .done1
            cmp eax, 9          ; check if not a number
            jg .done1

            add r14, 1          ; next symbol
            add rbx, 1          ; increase rbx
            jmp .next

.done1:     sub rbx, 1          ; decrease rbx for power of 10 working
            pop r14             ; restore read buffer
            ret


;--------------------------------------
; Entry: RDI = destination pointer
; Exit: RDI = input num
; Expects: R14 is the read buffer ptr
; Destroys: RAX, RCX, R8
;--------------------------------------

read_buf:
.next:      mov eax, [r14]      ; symbol in eax
            mov dl, al
            xor eax, eax        ; symbol in al
            mov al, dl
            sub eax, '0'        ; convert to num
            cmp eax, 0
            jl .done            ; checking if not a number
            cmp eax, 9
            jg .done            ; checking if not a number
            mov ch, 10          ; ch = 10 for the power of 10

            push rbx            ; saving remaining len
.pow:       cmp rbx, 0
            je .don             ; loop for mupltiplying number and power of 10
            mul ch
            sub rbx, 1
            jmp .pow

.don:       pop rbx             ; restore remaining len
            sub rbx, 1          ; next symbol
            add r8, rax         ; adding to the result
            add r14, 1          ; next symbol
            jmp .next           ; handling next symbol
            .done:

            ret
