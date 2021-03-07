; (*) start from i=0
; (*)if drone (i%N)+1 is active
;     (*) switch to the i’th drone co-routine
; (*) if i%K == 0 //time to print the game board
;     (*) switch to the printer co-routine
; (*) if (i/N)%R == 0 && i%N ==0 //R rounds have passed
;     (*) find M - the lowest number of targets destroyed, between all of the active drones
;     (*) "turn off" one of the drones that destroyed only M targets.
; (*) i++
; (*) if only one active drone is left
;     (*)print The Winner is drone: <id of the drone>
;     (*) stop the game (return to main() function or exit)


section	.rodata	 ; we define (global) read-only variables in .rodata section
	format_string_NL: db "%s",10, 0	; format string
    format_decimal_NL: db "%d", 10, 0	; format string
    format_float_NL: db "%f", 10, 0
    format_winner: db "The Winner is drone: %d",10,0
    format_id: db "@@@@@@@drone %d turns@@@@@@@@",10,0
    format_elimination: db "elimination round!",10,0
    format_loop_start: db "#########starting loop!############ ",10,0
    format_step: db "%%%% STEP NUMBER: %d %%%%%%",10,0
    format_eliminated: db "ELIMINITAED DRONE: %d ",10,0
    str: db "***",10,0

section .data

    cor_struct: dd 0

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
    MAXINT: equ 2147483647


    mod_N: dd 0
    mod_K: dd 0
    mod_R: dd 0

    lowest_hits: dd MAXINT
    eliminated_drone: dd 0
    active_drones: dd 0

    ; first_call_to_scheduler: dd 0

    ;main_ret_addr: dd 0

section .text
    extern corsPtr
    global foo
    extern printf
    extern get_random
    extern seed
    global createTarget
    extern res
    extern prev_cor
    extern dronesNum
    extern curr_cor
    extern resume
    extern printer_cor
    extern roundsNumBetweenElimination
    extern stepsNumBetweenPrint
    global scheduler
    extern main_esp
    extern target_cor
    extern main_ret_addr
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

    %macro get_cor_struct 1
        pushad
        mov eax, dword[corsPtr]
        mov ebx, %1
        shl ebx, 2
        add eax, ebx
        mov ecx, dword[eax]
        mov dword[cor_struct], ecx
        popad
    %endmacro

    %macro get_Mod_R 1
        pushad
        mov eax, %1
        mod_R_loop:
            cmp eax, dword[roundsNumBetweenElimination]
            jl end_loop
            sub eax, dword[roundsNumBetweenElimination]
            jmp mod_R_loop
        end_loop:
        mov dword[mod_R], eax
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

    %macro print_winner 0
        pushad                                               
        mov edx, dword[corsPtr]             ;poindter to to cors array
        winner_loop:
            mov ebx, dword[edx]             ;curr drone
            cmp dword[ebx+active_offset], 1
            je %%print
            add edx, 4    
            jmp winner_loop
        %%print:
            mov eax , dword[debug_mode]
            mov dword[debug_mode], 1
            print2 dword[ebx+id_offset], format_winner
            mov dword[debug_mode], eax
        popad
    %endmacro

    
    %macro printEliminated 0
        print2 dword[eax+id_offset], format_eliminated
    %endmacro
    ;round = N steps
    scheduler:
    ;push dword[target_cor]                          ;init target
    mov ebx, dword[target_cor]
    call resume
    mov eax, dword[dronesNum]                      ;set number of active drones
    mov dword[active_drones], eax   
    mov eax, 0                                     
    mov ecx, 0                                      ;ecx = i                                
    mov edx, 0                                    
    mov ebx, 0       

    print1 format_loop_start                            
    scheduler_loop:
        print2 ecx, format_step
        mov edx, dword[mod_N]                       ; get i%N
        cmp edx, dword[dronesNum]                   ;handel mod N
        jl mod_N_ok
        sub edx, dword[dronesNum]
        mov dword[mod_N], edx
        mod_N_ok:
        print2 dword[mod_N],format_id
        check_if_active:
        get_cor_struct edx                          ;cor_struct = cors[i%N]
        mov eax, dword[cor_struct]                  ;eax = curr drone 
        cmp dword[eax+active_offset], 0             ; is Active?
        je check_print
        ;push eax    
        mov ebx, eax                           
        call resume                                 ;Active? resume(cur_drone)
        check_print:                                ;handle Mod K
        mov ebx, dword[mod_K]                       ;get i%K
        cmp ebx, dword[stepsNumBetweenPrint]
        jl mod_K_ok
        sub ebx, dword[stepsNumBetweenPrint]
        mov dword[mod_K], ebx
        mod_K_ok:
        check_print_time:
        mov ebx, dword[mod_K]   
        cmp ebx, 0                                  ;TO Print?
        ja check_R
        print:
        ;push dword[printer_cor]                     ;if we have to print call resume(printer)
        mov ebx, dword[printer_cor] 
        call resume
        check_R:    ;we have to check : i/N > 0  &&  (i/N)%R == 0 && 1%N == 0 ?
        mov eax, ecx                                ;eax = i
        div dword[dronesNum]                        ;eax = i/N
        cmp eax, 0                                  ; (i/N)>0 ? first round not elimination
        je not_elimination_round
        get_Mod_R eax                               ;mod_R = (i/N)%R
        cmp dword[mod_R], 0                         ;elimination round ?
        jne not_elimination_round
        cmp dword[mod_N], 0                         ;i%N == 0 ? eliminate once every N steps
        jne not_elimination_round
        print1 format_elimination
        call get_lowest_hits_and_eliminate          ;eliminate and maintain active drones 
        not_elimination_round:
        cmp dword[active_drones], 1                 ;last drone? winner?
        jne no_winner_yet
        finish_game:  ;stop the game return to main() by CALL END_COR
        print_winner  
        push dword[main_ret_addr]
        ret
        no_winner_yet:
        inc ecx
        inc dword[mod_N]
        inc dword[mod_K]
    jmp scheduler_loop


get_lowest_hits_and_eliminate:
    startFunc
    mov dword[lowest_hits], MAXINT  
    mov ecx, dword[dronesNum]                                    
    mov ebx, dword[corsPtr]
    min_hits_loop:
    mov eax, dword[ebx]             ;curr drone
    cmp dword[eax+active_offset], 0
    je .continue                    ;if not active- dont care
    mov edx, dword[eax+hits_offset] ;get #hits
    cmp edx, dword[lowest_hits]     ;cur_hits < lowest_hits ?   
    jae .continue
    mov dword[lowest_hits], edx     ;if true => update minimum
    mov dword[eliminated_drone], eax 
    .continue:
    add ebx, 4   
    loop min_hits_loop, ecx
    eliminate_drone:
    mov eax, dword[eliminated_drone]
    mov dword[eax+active_offset], 0
    printEliminated
    dec dword[active_drones]
    endFunc



    ; resume:                     ;save state of curr co_routine
    ;     mov ebx, [esp+4]        ;ebx target co-routine
    ;     check_skip              ;skip on the first time to do_resume
    ;     pushfd                  ;save state of caller
    ;     pushad
    ;     mov edx, dword[curr_cor]
    ;     mov [edx+mini_stack_offset], esp      ;save current sp/mini stack
    ; do_resume:                  ;load esp for resumed co routine
    ;     mov esp, [ebx+mini_stack_offset]      ;load sp (resumed cor)
    ;     mov dword[prev_cor], edx                ;maintain prev_cor
    ;     mov dword[curr_cor], ebx
    ;     popad                   ;restore resumed co routine state
    ;     popfd
    ;     ret                     ;return to resumed co routine

