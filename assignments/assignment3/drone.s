; (*) Generate random heading change angle  ∆α       ; generate a random number in range [-60,60] degrees, with 16 bit resolution
; (*) Generate random speed change ∆a         ; generate random number in range [-10,10], with 16 bit resolution        
; (*) Compute a new drone position as follows:
;         (*) first, move speed units at the direction defined by the current angle, wrapping around the torus if needed. 
;         For example, if speed=60 then move 60 units in the current direction.
;     (*) then change the current angle to be α + ∆α, keeping the angle between [0, 360] by wraparound if needed
;     (*) then change the current speed to be speed + ∆a, keeping the speed between [0, 100] by cutoff if needed
; (*) Do forever
;     (*) if mayDestroy(…) (check if a drone may destroy the target)
;         (*) destroy the target	
;         (*) resume target co-routine
;     (*) Generate random angle ∆α       ; generate a random number in range [-60,60] degrees, with 16 bit resolution
;     (*) Generate random speed change ∆a    ; generate random number in range [-10,10], with 16 bit resolution        
;     (*) Compute a new drone position as follows:
;         (*) first, move speed units at the direction defined by the current angle, wrapping around the torus if needed. 
;         (*) then change the new current angle to be α + ∆α, keeping the angle between [0, 360] by wraparound if needed
;         (*) then change the new current speed to be speed + ∆a, keeping the speed between [0, 100] by cutoff if needed
;     (*) resume scheduler co-routine by calling resume(scheduler)	
; (*) end do	

section	.rodata	 ; we define (global) read-only variables in .rodata section
	format_string_NL: db "%s",10, 0	; format string
    format_decimal_NL: db "%d", 10, 0	; format string
    format_float_NL: db "%.2f", 10, 0
    format_float: db "%.2f    ", 0
    format_angle_change: db "angle change: ",0
    format_speed_change: db "speed change: ",0
    format_angle: db "angle : ",0
    format_speed: db "speed : ",0
    format_drone_start: db "^^^^^^^^^ INSIDE A DRONE CO! ^^^^^^^^",10,0
    format_drone_status: db "^^^^^^^^^ STATUS OF A DRONE CO! ^^^^^^^^",10,0
    format_destroy: db "$$$$$$$$$ TARGET DESTROYED!!!!! ########",10,0
    format_id: db "@@@@@@@drone %d @@@@@@@@",10,0
    format_target: db "X            Y",10,0
    format_myLoc: db "MY LOCATION:",10,0
    format_TargetLoc: db "TARGETS LOCATION:",10,0
    str: db "***",10,0
    blabla: db "@@@",10,0


section .data
    speed: dd 0.0
    angle: dd 0.0
    range: dd 0
    mid: dd 60
    MAXSZ: dd 65535
    n180: dd 180.0
    n100: dd 100.0
    n0: dd 0.0
    n360: dd 360.0
    canDestroy: dd 0
    temp: dd 0.0

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

section .text
    global generateAngleChange
    global generateSpeedChange

    extern corsPtr
    extern printf
    extern get_random
    extern seed
    extern res
    extern target_y
    extern target_x
    extern destroyRange
    extern scheduler_cor
    extern target_cor
    extern curr_cor
    extern resume
    global drone
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

        %macro print4 2
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

    %macro print3 1
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




    %macro printLoc 0
        print1 format_myLoc
        print1 format_target
        print_fpoint dword[eax+x_offset],format_float
        print_fpoint dword[eax+y_offset],format_float_NL
    %endmacro

    %macro printTargetLoc 0
        print1 format_TargetLoc
        print1 format_target
        print_fpoint dword[target_x],format_float
        print_fpoint dword[target_y],format_float_NL
    %endmacro

    %macro printSpeed 0
        print1 format_speed
        print_fpoint dword[eax+speed_offset], format_float_NL
    %endmacro

    %macro printAngle 0
        print1 format_angle
        print_fpoint dword[eax+direction_offset], format_float_NL
    %endmacro
    %macro printSpeedChange 0
        print1 format_speed_change
        print_fpoint dword[speed], format_float_NL
    %endmacro

    %macro printAngleChange 0
        print1 format_angle_change
        print_fpoint dword[angle], format_float_NL
    %endmacro

    ; a b dest
    %macro generateInRange 3 
        pushad
        call get_random         ;get random num to seed
        ;finit
        fild dword[seed]        ; load rand
        mov dword[range], %2    ; range = b - a
        sub dword[range], %1
        fidiv dword[MAXSZ]      ;rand/MAX
        fimul dword[range]     ;(rand/MAX)*range
        mov dword[mid], %1
        fiadd dword[mid]       ; 

        fstp %3
        ;ffree
        popad
    %endmacro

    generateAngleChange:
        startFunc
        generateInRange -60, 60, dword[angle]
        printAngleChange        
        endFunc

    generateSpeedChange:
        startFunc
        generateInRange -10, 10, dword[speed]
        printSpeedChange       
        endFunc

    generateAngle:
        startFunc
        generateInRange 0, 360, dword[eax+direction_offset]
        endFunc

    generateSpeed:
        startFunc
        generateInRange 0, 100, dword[eax+speed_offset]
        endFunc


    drone:  
        finit
        mov eax, dword[curr_cor] 
        fld dword[n0]                       
        fstp dword[eax+x_offset]                ;init drone's location as 0,0
        fld dword[n0]  
        fstp dword[eax+y_offset]  
        fld dword[n0]  
        fstp dword[eax+direction_offset] 
        fld dword[n0]  
        fstp dword[eax+speed_offset]

        call generateSpeedChange
        call generateAngleChange
        
        mov ebx, dword[angle]                   ;get generated angle

        mov dword[eax+direction_offset],ebx     ;init drone's direction as angle
        mov ebx, dword[speed]                   ;get generated speed
        mov dword[eax+speed_offset],ebx         ;init drone's speed
        call update_position 
        forever_loop:

            print1 format_drone_start
            print2 dword[eax+id_offset],format_id
            printLoc
            print1 str
            printTargetLoc
            print1 str

            call mayDestroy
            cmp dword[canDestroy], 0
            je .continue
            mov eax, dword[curr_cor]
            inc dword[eax + hits_offset]

            print1 format_destroy           

            ;push dword[target_cor]
            mov ebx, dword[target_cor]
            call resume
            .continue:
                call generateSpeedChange
                call generateAngleChange
                call update_position

                ;push dword[scheduler_cor]
                mov ebx, dword[scheduler_cor]
                call resume
        jmp forever_loop

    mayDestroy: 
                            ;calc distance = (  (x1-x2)^2 + (y1-y2)^2  ) ^ 0.5
        startFunc

        mov eax, dword[curr_cor]  
        mov dword[canDestroy], 0
        ;finit
        fld dword[target_x]            ;load x1
        fld dword[eax+x_offset]
        fsubp        ;x1-x2
        fld dword[target_x]            ;load x1
        fld dword[eax+x_offset]       ;x1-x2
        fsubp 
        fmulp                            ;(x1-x2)^2
        fld dword[target_y]            ;same for y
        fld dword[eax+y_offset]
        fsubp 
        fld dword[target_y]
        fld dword[eax+y_offset] 
        fsubp       
        fmulp                            ;(y1-y2)^2
        faddp                            ;(x1-x2)^2 + (y1-y2)^2
        fsqrt                           ;( (x1-x2)^2 + (y1-y2)^2 ) ^0.5 = D
        fld dword[destroyRange]         
        fcomip                          ;D > destroyRange ?
        jc .finish                      ;jump if D > destroyRange
        mov dword[canDestroy], 1        ;if not - return True
        .finish:
        ;ffree
        endFunc

    update_position:
        startFunc
        mov eax, dword[curr_cor] 
        ;ffree
        finit
        fld dword[eax+direction_offset]    ;load dir
        convert_to_radians_Y:                 ; = dir*pi/180
        fldpi                               ;load pi
        fmulp                               ;dir*pi
        fld dword[n180]                    ;load 180
        fdivp                               ;dir/180
        calc_new_loc:
        ;in stack- value of dir in radians
        fsin                             ;sin(dir)
        update_x_y:
        update_y:
        fld dword[eax+speed_offset]         ;load speed 
        fmulp                               ;sin(dir)*speed = C
        fld dword[eax+y_offset]             ;load y
        faddp                               ;y' = y + C
        Y_mod100_check:
        fld  dword[n100]                    ;y' > 100?
        fcomip
        ja Y_lessThan100                    ;jump if y' < 100    
        fld  dword[n100]                   ;y' = y' - 100
        fsubp
        jmp Y_ok   
        Y_lessThan100:
        fld  dword[n0]                      ;y' < 0 ?
        fcomip
        jc Y_ok                             ;jump if y' > 0
        jz Y_ok                             ;jump if y' = 0
        Y_lessThan0:
        fld  dword[n100]                   ;y' = y' + 100
        faddp
        Y_ok:
        fstp dword[eax+y_offset]            ;y = y'
        ;stack is empty
        update_X:
        fld dword[eax+direction_offset]    ;load dir
        convert_to_radians_X:                 ; = dir*pi/180
        fldpi                               ;load pi
        fmulp                               ;dir*pi
        fld dword[n180]                    ;load 180
        fdivp                               ;dir/180
        ;in stack- value of dir in radians
        fcos 
        fld dword[eax+speed_offset]         ;load speed 
        fmulp                               ;cos(dir)*speed = B
        fld dword[eax+x_offset]             ;load x
        faddp                               ;x' = x + B
        X_mod100_check:
        fld  dword[n100]             
        fcomip
        ja X_lessThan100                  ;jump if x' < 100
        fld dword[n100]                   ;x' = x' - 100
        fsubp 
        jmp X_ok   
        X_lessThan100:
        fld  dword[n0]                      ;x' < 0 ?
        fcomip
        jc X_ok                             ;jump if x' > 0
        jz X_ok                             ;jump if x' = 0
        fld dword[n100]                   ;x' = x' + 100
        faddp 
        X_ok:
        fstp dword[eax+x_offset]            ;x = x'
        ; ffree
        ; finit
        ;stack is empty!!!
        update_dir:
        fld dword[eax+direction_offset]
        fld dword[angle]
        faddp                             ;dir' = dir + angle
        dir_mod360_check:   
        fld dword[n360]
        fcomip
        ja dir_lessThan360                ;jump if dir' < 360
        fld  dword[n360]
        fsubp
        jmp dir_ok
        dir_lessThan360:
        fld  dword[n0]                      ;dir' < 0 ?
        fcomip
        jc dir_ok                           ;jump if dir' > 0
        jz dir_ok                           ;jump if dir' = 0
        fadd  dword[n360]                   ;dir' = dir' + 360
        dir_ok:
        fstp dword[eax+direction_offset]


        update_speed: ; FIXED
        fld dword[eax+speed_offset]
        fld dword[speed]
        faddp

        speed_check: 
        fld dword[n100]
        fcomip

        jb speed_moreThan100              ;jump if speed' > 100
        fld dword[n0]
        fcomip
        ja speed_lessThan0           ;jump if speed' < 0
        jmp speed_in_range

        speed_moreThan100:
        fld dword[n100] 
        jmp speed_in_range

        speed_lessThan0:
        fld dword[n0] 
        jmp speed_in_range

        speed_in_range:
        fstp dword[eax+speed_offset]
        print1 format_drone_status
        printLoc
        printSpeed
        printAngle
        print2 dword[eax+hits_offset], format_decimal_NL

        ffree
        endFunc
