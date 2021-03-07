; (*) print the game board according to the format described below
; (*) switch back to a scheduler co-routine



section	.rodata	 ; we define (global) read-only variables in .rodata section
    format_string: db "%s", 0	; format string
	format_string_NL: db "%s",10, 0	; format string
    format_decimal: db "%d , ", 0	; format string
    format_decimal_NL: db "%d",10, 0	; format string
    format_float: db "%.2f ,", 0
    format_float_NL: db "%.2f ",10, 0
    format_NL: db "",10, 0
    format_printing: db "******printing time!********",10,0
    format_vals: db "ID    X      Y      dir    speed hits",10,0
    format_target: db "X      Y",10,0
    format_TargetLoc: db "TARGETS LOCATION:",10,0

section .data
    speed: dd 0
    angle: dd 0
    angleRange: dd 120
    MAXSZ: dd 65535
    halfAngleRange: dd 60

    speedChange: dd 20
    midspeedChange:dd 10

    ptrSize: equ 4
    structSize: equ 37 ;4*4(x,y,direction,speed) + 5*4 (code,flags,spp,#hits,id)+1(active)
    x_offset: equ 0
    y_offset: equ 4
    direction_offset: equ 8
    speed_offset: equ 12
    code_offset: equ 16
    flags_offset: equ 20
    mini_stack_offset: equ 24
    hits_offset: equ 28
    id_offset: equ 32
    active_offset: equ 36
    mini_stack_size: equ 16*1024

    n180: dd 180
    n100: dd 100
    n0: dd 0
    n360: dd 360
    canDestroy: dd 0

section .text
    extern corsPtr
    extern printf
    extern get_random
    extern seed
    global createAngle
    global changeSpeed
    extern res
    extern target_y
    extern target_x
    extern destroyRange
    extern scheduler_cor
    extern target_cor
    extern curr_cor
    extern resume
    extern dronesNum
    global printer

    %macro print2 2 
        pushad
        pushfd
        push %1
        push %2
        call printf
        add esp, 8
        popfd
        popad
    %endmacro

    %macro startFunc 0
        pushad
        push ebp                ; backup ebp
        mov ebp, esp            ; set ebp to func activaion frame
        sub esp,4               ; allocate for output
    %endmacro

    %macro endFunc 0
        mov esp, ebp    ; “free” function activation frame
        pop ebp         ; restore activation frame of main()
        popad
        ret
    %endmacro

    ;input: number and formet 
    %macro print_fpoint 2
        pushad
        finit
        fld %1
        fstp qword[res]
        push dword[res+4]
        push dword[res]
        push %2
        call printf
        add esp, 12 
        ffree
        popad
    %endmacro

    %macro print1 1
        pushad
        pushfd
        push %1
        call printf
        add esp, 4
        popfd
        popad
    %endmacro

        %macro printLoc 0
        ;print1 format_TargetLoc
        ;print1 format_target
        print_fpoint dword[target_x],format_float
        print_fpoint dword[target_y],format_float_NL
    %endmacro

;check in which format we have to print each value
    ; printer:
    ;     print1 format_NL
    ;     print1 format_NL
    ;     ;print1 format_printing
    ;     print_target:
    ;         printLoc
    ;     print_drones:
    ;     ;print1 format_vals
    ;     mov eax, dword[corsPtr]
    ;     mov ebx, 0
    ;     mov ecx, dword[dronesNum]
    ;     print_loop:
    ;     mov ebx, dword[eax]
    ;     cmp dword[ebx+active_offset],0
    ;         je .continue
    ;     print2 dword[ebx+id_offset], format_decimal
    ;     print_fpoint dword[ebx+x_offset], format_float  
    ;     print_fpoint dword[ebx+y_offset], format_float  
    ;     print_fpoint dword[ebx+direction_offset], format_float 
    ;     print_fpoint dword[ebx+speed_offset], format_float  
    ;     print2 dword[ebx+hits_offset], format_decimal_NL  
    ;     print1 format_NL
    ;     .continue:
    ;     add eax, 4
    ;     dec ecx
    ;     cmp ecx, 0
    ;     jne print_loop
    ;     ;push dword[scheduler_cor]
    ;     mov ebx, dword[scheduler_cor]
    ;     call resume
    ; jmp printer

        printer: ;print1 format_printing 
        print_target: 
        printLoc 
        print_drones: 
        ;print1 format_vals
        mov eax, dword[corsPtr] 
        mov ebx, 0 
        mov ecx, dword[dronesNum]
        print_loop:
            mov ebx, dword[eax] 
            cmp dword[ebx+active_offset],0 
            je .continue 
            print2 dword[ebx+id_offset], format_decimal
            print_fpoint dword[ebx+x_offset], format_float
            print_fpoint dword[ebx+y_offset], format_float
            print_fpoint dword[ebx+direction_offset], format_float
            print_fpoint dword[ebx+speed_offset], format_float
            print2 dword[ebx+hits_offset], format_decimal_NL
            print1 format_NL 
            .continue: add eax, 4
            dec ecx 
            cmp ecx, 0
        jne print_loop
        ;push dword[scheduler_cor] 
        mov ebx, dword[scheduler_cor] 
        call resume 
        jmp printer 



    

