; *) call createTarget() function to create a new target with random coordinates on the game board
; (*) switch to the co-routine of the "current" drone by calling resume(drone id) function



section	.rodata	 ; we define (global) read-only variables in .rodata section
	format_string_NL: db "%s",10, 0	; format string
    format_decimal_NL: db "%d", 10, 0	; format decimal
    format_decimal: db "%d  ", 0	; format decimal
    format_float_NL: db "%.2f", 10, 0
    format_float: db "%.2f ", 0
    format_target: db "X       Y",10,0
    format_target_start: db "^^^^^^^^^ INSIDE THE TARGET CO! ^^^^^^^^",10,0
    format_target_end: db "^^^^^^^^^FINISH THE TARGET CO! ^^^^^^^^",10,0
    str: db "***",10,0
section .data
    global target_x
    target_x: dd 0.0
    global target_y
    target_y: dd 0.0
    MAXSZ: dd 65535
    range: dd 0
    mid: dd 0

section .text
    extern corsPtr
    extern printf
    extern get_random
    extern seed
    global generateNewTarget
    extern res
    extern prev_cor
    extern resume
    global target
    extern debug_mode

    %macro print2 2
        cmp dword[debug_mode],1
        jne %%continue 
        pushad
        pushfd
        push %1
        push %2
        call printf
        add esp, 8
        popfd
        popad
        %%continue:
    %endmacro

    %macro print1 1
        cmp dword[debug_mode],1
        jne %%continue 
        pushad
        pushfd
        push %1
        call printf
        add esp, 4
        popfd
        popad
        %%continue:
    %endmacro

    ;input: number and formet 
    %macro print_fpoint 2
        cmp dword[debug_mode],1
        jne %%continue 
        pushad
        fld %1
        fstp qword[res]
        push dword[res+4]
        push dword[res]
        push %2
        call printf
        add esp, 12 
        popad
        %%continue:
    %endmacro

    %macro startFunc 0
        push ebp                ; backup ebp
        mov ebp, esp            ; set ebp to func activaion frame
        sub esp,4               ; allocate for output
    %endmacro

    %macro endFunc 0
        mov esp, ebp    ; “free” function activation frame
        pop ebp         ; restore activation frame of main()
        ret
    %endmacro

;input [a,b] destination
    %macro generateInRange 3 
        pushad
        call get_random         ;get random num to seed
        finit
        fild dword[seed]        ; load rand
        mov dword[range], %2    ; range = b
        sub dword[range], %1    ; range = b-a
        fidiv dword[MAXSZ]      ;random/MAX
        fimul dword[range]     ;(rand/MAX)*range
        mov dword[mid], %1
        fiadd dword[mid]       
        fstp %3
        ffree
        popad
    %endmacro
    ; (*) call createTarget() function to create a new target with random coordinates on the game board
    ; (*) switch to the co-routine of the "current" drone by calling resume(drone id) function


    target:
        print1 format_target_start
        call generateNewTarget
        print1 format_target_end
        ;push dword[prev_cor]
        mov ebx, dword[prev_cor]
        call resume
    jmp target

    generateNewTarget:
        startFunc
        generateInRange 0,100, dword[target_x]
        generateInRange 0,100, dword[target_y]
        print1 format_target
        print_fpoint dword[target_x],format_float
        print_fpoint dword[target_y],format_float_NL
        endFunc
    
