section .text
global _start
global _printf

;==============================================================================
; outputs the string in the format specified by the user
;==============================================================================
; ENTRY: printfBuf - buffer for write into;
;        rdi = num 
;
; EXIT:  none  

; DESTROY: rax, rcx
;==============================================================================
_printf: 
        mov rdx, rdi
        mov rdi, r11 ; printfBuf = r11

        push rdi
        mov rdi, r12  ; itoaBuf = r12

        call itoa10

        pop rdi

        push rsi

        mov rsi, r12
        call cpy2buf

        pop rsi

        mov byte [rdi], 10 
        inc rdi
        mov byte [rdi], 0

        call resetBuf
        ret
 
;==============================================================================
; displays the current state of the buffer, and then clears it 
;==============================================================================
; ENTRY: printfBuf - buffer for printf 
;        rdi       - position of last byte;
;
; EXIT:  rdi = printfBuf  

; DESTROY: 
;==============================================================================
resetBuf:
        push rax
        push rdx
        push rsi
        
        mov rdx, rdi
        mov rax, 0x01 
        mov rdi, 1
        mov rsi, r11
        sub rdx, r11
        syscall

        pop rsi
        pop rdx
        pop rax

        mov rdi, r11 

        ret
;==============================================================================
; copy string into buffer
;==============================================================================
; ENTRY: rdi - buffer for write into;
;        rsi - string pointer
;
; EXIT:  none  

; DESTROY: rax, rdi, rsi
;==============================================================================
cpy2buf:

.cpy_loop:
        lodsb 

        cmp al, 0x00 
        je .done 

        stosb
        jmp .cpy_loop

.done:
        mov byte [rdi], 0x00 
        ret
;==============================================================================

       
;==============================================================================
; converts numbr into string (in base 10), write it in buf
;==============================================================================
; ENTRY: rdx - int value;
;        rdi - buf to write into
;
; EXIT:  r13 - the count of written symbols

; DESTROY: r8, r9, rdx
;==============================================================================
itoa10:

        xor r13, r13
%if 1
        cmp edx, 0
        jge .next

.negative:
        mov byte [rdi], '-' 
        inc rdi
        inc r13

        neg edx
%endif 

.next:
        mov r8, rdx
        mov rax, rdx 
        mov r9, 0x0a

.count_len:
        xor rdx, rdx
        div r9

        inc rdi
        inc r13
        
        cmp rax, 0x00 
        ja .count_len

        mov rax, r8 
        mov byte [rdi], 0x00 
        dec rdi 

.write_num: 
        xor rdx, rdx
        div r9

        add dl, '0'
        mov [rdi], dl
        dec rdi 

        cmp rax, 0x00 
        ja .write_num 

        inc rdi

        ret
;==============================================================================


section .bss
buf_size equ 0x100 
printfBuf resb buf_size
itoaBuf resb 0x100


