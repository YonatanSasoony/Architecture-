section	.rodata	 ; we define (global) read-only variables in .rodata section
	format_string_NL: db "%s ",10, 0	; format string
    format_decimal_NL: db "%d ", 10, 0	; format string
    format_float_NL: db "%.2f ", 10, 0
    format_decimal: db "%d ",0 ; string for sscanf********
    format_float: db "%f ",0 ; string for sscanf*****
    format_NL: db " ",10,0 
    format_start: db "game started!",10,0
    format_PrintArgs: db "N R K     d  seed",10,0
    format_short_NL: db "%u ",10,0 
    str: db "***", 10, 0
    bla: db "##",10,0

section .data
    operandsNum: dd 0
    temp: dd 0
    global dronesNum
    dronesNum: dd 0
    global roundsNumBetweenElimination
    roundsNumBetweenElimination: dd 0
    global stepsNumBetweenPrint
    stepsNumBetweenPrint: dd 0
    global destroyRange
    destroyRange: dd 0.0
    global seed
    seed: dd 0
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
    global res
    res:dq 0
    global curr_cor
    curr_cor: dd 0
    global prev_cor
    prev_cor: dd 0
    global scheduler_cor
    scheduler_cor: dd 0
    global target_cor
    target_cor: dd 0
    global printer_cor
    printer_cor: dd 0
    temp_esp: dd 0
    global main_esp
    main_esp: dd 0
    corsNum: dd 0
    n60: dd 60.0
    n360: dd 360.0
    n0: dd 0.0
    n100: dd 100.0    
    global debug_mode
    debug_mode: dd 0

    first_call_to_scheduler: dd 0
    global main_ret_addr
    main_ret_addr: dd 0

section .bss	; we define (global) uninitialized variables in .bss section
    argsBuffer: resb 20
    global corsPtr
    corsPtr: resb 4
    structPtr: resb 4
    miniStackPtr: resb 4
    SPPARR: resb 4
    

section .text
    align 16
    global main
    global get_random
  
    extern printf
    extern fprintf 
    extern malloc 
    extern calloc ;;;;; buffer[],, "%d %d %d %s" , &numOFdgf ,sdgs sgs) 
    extern free 
    extern sscanf
    extern stdin
    
    extern generateAngleChange
    extern generateSpeedChange
    extern generateNewTarget

    extern scheduler
    extern target
    extern drone
    extern printer
    global resume


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

    %macro scan 3
        pushad
        pushfd
        push %3
        push %2
        push %1
        call sscanf
        add esp, 12
        popfd
        popad
    %endmacro
    
    %macro getArgs 0
        mov eax, dword[esp+8]   ;pointer to the zero arg (file name)

        add eax, 4
        ;print2 dword[eax], format_string_NL
        scan dword[eax], format_decimal, dronesNum
        ;print2 dword[dronesNum], format_decimal

        add eax, 4
        ;print2 dword[eax], format_string_NL
        scan dword[eax], format_decimal, roundsNumBetweenElimination
        ;print2 dword[roundsNumBetweenElimination], format_decimal_NL

        add eax, 4
        ;print2 dword[eax], format_string_NL
        scan dword[eax], format_decimal, stepsNumBetweenPrint
        ;print2 dword[stepsNumBetweenPrint], format_decimal_NL

        add eax, 4
        ;print2 dword[eax], format_string_NL
        scan dword[eax], format_float, destroyRange
        ;print_fpoint dword[destroyRange], format_float_NL
        ;print2 dword[destroyRange], format_float_NL
        
        add eax, 4
        ;print2 dword[eax], format_string_NL
        scan dword[eax], format_decimal, seed
        ;print2 dword[seed], format_decimal_NL
    %endmacro

    %macro printArgs 0
        print1 format_PrintArgs
        print2 dword[dronesNum], format_decimal
        print2 dword[roundsNumBetweenElimination], format_decimal
        print2 dword[stepsNumBetweenPrint], format_decimal
        print_fpoint dword[destroyRange], format_float_NL
        print2 dword[seed], format_decimal_NL
    %endmacro



    %macro createCorsArr 1
        pushad
        pushfd
        push %1
        push ptrSize         
        call calloc
        mov dword[corsPtr], eax
        add esp, 8
        popfd
        popad
    %endmacro

    %macro createSPPArr 0
        pushad
        pushfd
        push dword[corsNum]
        push ptrSize         
        call calloc
        mov dword[SPPARR], eax
        add esp, 8
        popfd
        popad
    %endmacro

    %macro createStruct 0
        pushad
        pushfd
        push structSize
        push 1        
        call calloc
        mov dword[structPtr], eax
        add esp, 8
        popfd
        popad
    %endmacro

    %macro createMiniStack 0
        pushad
        pushfd
        push mini_stack_size
        push 1        
        call calloc
        mov dword[miniStackPtr], eax
        add esp, 8
        popfd
        popad
    %endmacro
; input: cor id, mini stack ptr
    %macro saveSPP 2
        pushad
        mov ebx, %1     ;cor id
        mov ecx, %2     ; ptr
        mov eax, dword[SPPARR]
        shl ebx, 2      ; get pointer to location in array 
        add eax,ebx     ;eax points to arr[id]
        mov dword[eax],ecx      ;save ptr        
        popad
    %endmacro

;reserve memroty for all CORS
%macro initCors 0                   
        pushad
        pushfd
        createSPPArr        
        mov edx, 0                   ;id of cor
        mov ebx, dword[corsPtr]      ;pointer to cors array
        init_loop:
            createStruct                                        ;structPtr = new pointer from calloc
            mov ecx, dword[structPtr]   
            mov dword[ebx], ecx                                 ;set new pointer into corsPtr[i]
            mov dword[ecx+id_offset], edx                       ;set drone's id
            mov dword[ecx+code_offset], drone                   ;set code pointer
            mov dword[ecx+active_offset], 1                     ;set as asctive     
            createMiniStack                                     ;miniStackPtr = new pointer to mini stack from calloc     
            mov eax, dword[miniStackPtr]
            saveSPP edx, eax
            mov dword[ecx+mini_stack_offset], eax               ;set miniStackPtr into struct
            add dword[ecx+mini_stack_offset], mini_stack_size   ;for pointing to the highest addr located in memroy 
            inc edx                                             ;id++
            add ebx, 4                                          ;i++
            cmp edx,dword[corsNum]
            jne init_loop

    init_special_cors:
        mov ebx, dword[corsPtr]                   ;get arr cors array
        mov ecx, dword[dronesNum]                 ;get drones num
        shl ecx, 2                                ;#drones * 4 to get to the special cors
        add ebx, ecx                              ;point to the scheduler
        mov eax, dword[ebx]                       ;get scheduler_cor
        mov dword[scheduler_cor], eax       
        mov dword[eax+code_offset], scheduler     ;set code pointer
        add ebx, 4
        mov eax, dword[ebx]                       ;get target_cor
        mov dword[target_cor], eax
        mov dword[eax+code_offset], target        ;set code pointer
        add ebx, 4
        mov eax, dword[ebx]                       ;get printer_cor
        mov dword[printer_cor], eax         
        mov dword[eax+code_offset], printer       ;set code pointer

        popfd
        popad
    %endmacro

    %macro init_mini_stack 0
        pushad
        mov ecx, dword[corsNum]
        mov edx, dword[corsPtr]                         ;poindter to to cors array
        %%init_loop:
            mov ebx, dword[edx]                         ;curr drone
            mov eax, dword[ebx+code_offset]             ;get pointer to code
            mov dword[temp_esp], esp                    ;back up esp   
            mov esp,dword[ebx+mini_stack_offset]        ;update esp to point on mini stack
            mov ebp, esp                                ;for good practice
            push eax                                    ;push inital return address 
            pushfd                                      ;push flags
            pushad                                      ;push regs
            mov dword[ebx+mini_stack_offset], esp       ;update mini stack pointer
            mov esp, dword[temp_esp]                    ;restore old esp
            add edx, 4                                         
        loop %%init_loop, ecx
        popad
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
        

    %macro printRandoms 0
        call get_random
        print2 word[seed], format_decimal_NL
        call get_random
        print2 word[seed], format_decimal_NL
        call get_random
        print2 word[seed], format_decimal_NL
        call get_random
        print2 word[seed], format_decimal_NL
        print1 str
        mov ecx, 6
        angle_print_loop:
            call generateAngleChange
            print1 str
            loop angle_print_loop, ecx
        print1 str
        print1 str
        mov ecx, 6
        speed_print_loop:
            call generateSpeedChange
            print1 str
            loop speed_print_loop, ecx
        print1 str
        print1 str
        mov ecx, 6
        traget_print_loop:
            call generateNewTarget
            print1 str
            loop traget_print_loop, ecx
    %endmacro

    %macro free_var 1
    pushad          
    pushfd
    push %1
    call free
    add esp, 4
    popfd
    popad
    %endmacro

    %macro free_all 0
    pushad
    mov eax, dword[corsPtr]
    mov ecx, dword[corsNum]
    mov edx, dword[SPPARR]
    free_loop:
    mov ebx, dword[eax]                    ;get curr cor  
    free_var dword[edx]                    ;free mini stack
    free_var ebx                           ;free struct
    add eax, 4
    add edx, 4
    loop free_loop, ecx
    free_cors_array:
    free_var dword[corsPtr]
    free_var dword[SPPARR]
    popad
    %endmacro

    %macro  check_skip 0
        inc dword[first_call_to_scheduler]
        cmp dword[first_call_to_scheduler], 1
        jne not_first_call
        pop dword[main_ret_addr]
        push dword[main_ret_addr]
        jmp do_resume
        not_first_call:
    %endmacro
        
    main:
        finit
        getArgs
        printArgs
        .continue:
        mov eax, dword[dronesNum]
        add eax,3
        mov dword[corsNum], eax
        createCorsArr dword[corsNum]
        initCors        
        init_mini_stack

        startCo:                            ;start game by calling scheduler
            pushad
            mov dword[main_esp], esp
            print1 format_start
            mov ebx, dword[scheduler_cor]
            ;push dword[scheduler_cor]
            call resume
        endCo:
            mov esp, dword[main_esp]
            popad
        ffree                               ;free X87
        free_all
    ret

    get_random:
        startFunc
        mov ecx,16
        random_loop:
            mov ax, word[seed] ; for xor calcs
            mov bx, word[seed] ;random num
            and ax, 45 ;45 is the number to isolate the taps bits- 0000000000101101
            jp insert_zero       ;if parity flag is on- the xor result is 0
            insert_one:
            shr bx,1            ;prepare for bit change
            or bx, 32768; 32768 is 2^15 - to change the msb bit
            jmp bit_ready
            insert_zero:
            shr bx,1
            bit_ready:
            mov word[seed],bx
        loop random_loop, ecx
        endFunc

    resume:                     ;save state of curr co_routine
        ;mov ebx, [esp+4]        ;ebx target co-routine
        check_skip              ;skip on the first time to do_resume
        pushfd                  ;save state of caller
        pushad
        mov edx, dword[curr_cor]
        mov [edx+mini_stack_offset], esp      ;save current sp/mini stack
    do_resume:                  ;load esp for resumed co routine
        mov esp, [ebx+mini_stack_offset]      ;load sp (resumed cor)
        mov dword[prev_cor], edx                ;maintain prev_cor
        mov dword[curr_cor], ebx
        popad                   ;restore resumed co routine state
        popfd
        ret                     ;return to resumed co routine
    