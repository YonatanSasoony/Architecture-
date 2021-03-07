section	.rodata	 ; we define (global) read-only variables in .rodata section
	prompt: db "calc: ", 0	; format string
	print_value: db "%s", 0	; format string
    print_decimal: db "%d", 10, 0	; format string
    print_hexa_without_pedding: db "%X", 0	; format string
    print_hexa: db "%02X", 0	; format string
    print_new_line: db "",10, 0
    print_stack_ptr: db "stackPtr: %X",10,0
    print_node_ptr: db "nodePtr: %X",10,0
    str: db "***", 10, 0
    printOverFlow: db  "Error: Operand Stack Overflow",10,0
    printNotEnoughNumbers: db "Error: Insufficient Number of Arguments on Stack",10,0
    ptrSize: dd 4
    defaultStackSize: dd 5
    helper: db "",0

section .data
    operandsNum: dd 0
    len: dd 0 
    total: db 0 
    curCapacity: dd 0
    curDigit: dd 0
    numberOfDigitsToPrint: dd 0
    temp: dd 0
    carry: dd 0 
    debugMode: dd 0
    

section .bss	; we define (global) uninitialized variables in .bss section
	buffer: resb 80
    stackPtr: resb 4
    stackHead: resb 4
    numPtr: resb 4
    lastPtr: resb 4
    op1: resb 4
    op2: resb 4
    result: resb 4
    start_result: resb 4
    stackSize: resb 4

section .text
  align 16
  global main
  extern printf
  extern fprintf 
  extern fflush
  extern malloc 
  extern calloc 
  extern free 
  extern gets 
  extern getchar 
  extern fgets 
  extern stdin
  extern stderr

%macro print1 1
    pushad
    pushfd
    push %1
    call printf
    add esp, 4
    popfd
    popad
%endmacro

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

%macro read 0
    pushad
    pushfd
    push dword [stdin]
    push dword 80
    push dword buffer
    call fgets
    add esp, 12
    popfd
    popad
%endmacro

%macro createStack 0
    pushad
    pushfd
    push dword [stackSize]
    push dword [ptrSize]
    call calloc 
    add esp, 8
    mov dword [stackPtr], eax
    mov dword[stackHead],eax
    popfd
    popad
%endmacro

%macro createNode 0
    pushad
    pushfd
    push dword 1
    push dword 5
    call calloc 
    mov dword[lastPtr], eax
    add esp, 8
    popfd
    popad
%endmacro

%macro insert_node_to_stack 0
    pushad
    pushfd
    mov ebx,0
    mov ebx,dword[stackPtr]
    mov eax,dword[lastPtr]
    mov dword[ebx], eax
    popfd
    popad
    %endmacro

%macro readSingleLetter 0

    mov eax, 0 
    mov al, byte [ecx]         ;read next digit to al
    cmp al, 57                  ; if al>9
    ja %%make_letter              ; convert letters A-F to 10-15
    sub al, 48                  ;convert from ascii to number
    jmp %%continue
    %%make_letter:
    sub al, 55
    %%continue: 
%endmacro

%macro minCapacityCheck 1
    cmp dword [curCapacity], %1
    jnb %%finish
    print1 printNotEnoughNumbers
    endFunc
    ret
    %%finish:
%endmacro

%macro maxCapacityCheck 0
    mov eax, dword [stackSize]
    cmp eax, dword [curCapacity]
    jne %%finish
    print1 printOverFlow
    endFunc
    ret
    %%finish:         
%endmacro

    
%macro freeList 0 

    pushad
    mov ebx, dword [stackPtr] ; get stackptr
    mov eax, dword [ebx]        ; eax = pointer to node
    mov edx ,0                  ;edx = counter

    %%push_pointers_loop:

        push eax                ;push pointer to stack
        inc edx                 ;count++
        mov ecx, eax            ;ecx = pointer to node
        inc ecx                 ;go to next pointer location
        mov eax, dword [ecx]    ;get next pointer

        cmp eax, 0              ;pointer = nullptr?
        jne %%push_pointers_loop    

        
    %%free_loop:
        pop eax     ;pop node pointer
        dec edx     ;count--

        pushad          ;free node
        pushfd
        push eax
        call free
        add esp, 4
        popfd
        popad

        cmp edx, 0          ;no more pointers to free
        jne %%free_loop
        mov eax, dword [ebx]        ; eax = pointer to node
        mov eax,0               ;reset the pointer to null
    popad
%endmacro

%macro freeStack 0
    pushad          ;free node
    pushfd
    push dword[stackHead]
    call free
    add esp, 4
    popfd
    popad

%endmacro


;input: char* HexString
%macro convertHexStringToInt 1
    pushad
    mov ecx, %1      ; ecx- pointer to file
    mov dword[total], 0 
    loop1:
        mov edx, 0
        cmp byte[ecx], 10
        je done1
        cmp byte[ecx], 0
        je done1
        
        shl dword[total], 4
        mov dl, byte[ecx]
      
        cmp dl, 57
        ja make_letter1
        sub dl, 48
        jmp dlReady
        make_letter1:
        sub dl, 55
      
        dlReady:
        add dword[total], edx
        inc ecx
        jmp loop1

        done1:
        popad
%endmacro

;set op1, op2, result and start_result to the relevant pointers
%macro setPointers 0
    mov eax, dword [stackPtr]
    mov ebx, dword [eax]    ;ebx= value of stack
    mov dword[op1], ebx     ; op1 = pointer to the first node 
    sub eax, 4
    mov ebx, dword [eax]    ;ebx= value of stack
    mov dword[op2], ebx     ; op2 = pointer to the second node
    createNode
    mov eax, dword [lastPtr]
    mov dword [start_result], eax   ;set start_result
    mov dword [result], eax         ;set result
%endmacro

;input: buffer
%macro print_debug_input 2

    cmp dword[debugMode],1
    jne %%end

    pushad
    pushfd
    mov eax, dword [stderr]

    push %1
    push %2
    push eax

    call fprintf
    add esp, 12
    popfd
    popad

    %%end:
%endmacro

;input: pointer to the result 
%macro print_debug_result 1

    cmp dword[debugMode], 1    ;debug?
    jne %%noPrint

    %%printNumber:
        mov ebx,0                   ;cur digit
        mov eax, %1                 ;pointer to the next node 
        mov ecx,0                   
        mov dword [numberOfDigitsToPrint], 0
       
        %%pushNumber_loop:
            mov bl,byte[eax]                ;the data of the node 
            push ebx 
            inc dword [numberOfDigitsToPrint]
            inc eax                         ;point to the node's pointer
            mov ecx, dword [eax]
            mov eax, ecx

            cmp eax, 0
            je %%print_first_without_pedding
            jne %%pushNumber_loop

        %%print_first_without_pedding:
            dec dword [numberOfDigitsToPrint]
            pop edx
            print_debug_input edx, print_hexa_without_pedding
            mov edx, 0

        %%printNumber_loop:

            cmp dword [numberOfDigitsToPrint], 0
            je %%finish_printing 

            dec dword [numberOfDigitsToPrint]
            pop edx
            print_debug_input edx, print_hexa
            mov edx, 0
            jmp %%printNumber_loop

        %%finish_printing:
            print_debug_input helper, print_new_line
        
         %%noPrint:

%endmacro

%macro startFunc 0
    push ebp                ; backup ebp
    mov ebp, esp            ; set ebp to func activaion frame
    sub esp,4               ; allocate for output
%endmacro

%macro endFunc 0
    mov esp, ebp    ; “free” function activation frame
    pop ebp         ; restore activation frame of main()
%endmacro

%macro getDebug 0
    
    
    mov ecx, dword[esp+4]    ;ecx = argc
    mov eax, [esp+8]  ;pointer to the zero arg (file name)

    cmp ecx, 1              ;no args
    je noDebug
    cmp ecx, 3              ;two args ?
    je yesDebug    

    add eax, 4                   ;pointer to the 1st arg
    mov edx, dword[eax]          ;get 1st argument
    cmp byte[edx], 45            ; debugMode?     ,   45 = ascii ( - )
    je yesDebug
    jmp noDebug

    noDebug:
        mov dword[debugMode], dword 0
        jmp finishDebug

    yesDebug:
        mov dword[debugMode], dword 1        

    finishDebug:
%endmacro

%macro getStackSize 0
    
    mov ecx, dword[esp+4]    ;ecx = argc

    cmp ecx, 1                      ;no args
    je noSize

    mov eax, dword[esp+8]        ;pointer to the zero arg (file name)
    add eax, 4                   ;pointer to the 1st arg
    mov edx, dword[eax]          ;get 1st argument
    cmp byte[edx], 45            ; debugMode? ,   45 = ascii ( - )
    jne getSize
    cmp ecx, 2  
    je noSize
    add eax,4 ;pointer to 2nd argument
    jmp getSize

    noSize:
        mov eax,dword[defaultStackSize] 
        mov dword[stackSize],eax            
        jmp finishSize

    getSize:
        mov edx, dword[eax]          ;get 2nd argument
        convertHexStringToInt edx   ;calculates the arg from hexa string to decimal into total 
        mov ebx, dword[total]
        mov dword[stackSize],ebx              
    finishSize:
%endmacro

%macro processArgs 0
  ;process args
    mov ecx, dword[esp+4]            ;ecx = argc
    cmp ecx, 1              ;no args
    je done

    mov eax, dword[esp+8]        ;pointer to the first arg
    add eax, 4

    cmp ecx, 3              ;two arg ?
    je two_args

    one_arg: 
    mov edx, dword[eax]
    cmp byte[edx], 45       ; debugMode?     ,   45 = ascii ( - )
    jne stack_size_arg 
    mov dword[debugMode], 1
    jmp done
    stack_size_arg:
    mov ebx,dword[eax]
    convertHexStringToInt ebx
    mov edx, dword[total]
    mov dword[stackSize], edx 
    jmp done

    two_args:
    mov dword[debugMode], 1     ;debug is on
    mov edx, dword[eax]
    cmp byte[edx], 45            ;debugMode?
    jne stack_size_arg          ;if not -> calc stack size
    add eax, 4                  ;else get the second arg (must be stacksize)
    jmp stack_size_arg          ;calc stack size

    done:
%endmacro

%macro removeLeadZero 1
    pushad
    mov eax, %1 ; get pointer to node ; eax = curr
    ;print2 eax, print_node_ptr

    mov ecx, eax ; ecx = prev
    inc eax ; get pointer to next node
    mov edx, dword[eax] ; get next node
    cmp edx, 0 ; next node = null?
    je %%finish_remove
    mov eax,edx ; eax = 2nd node; eax = curr 
    mov ebx, 0 ; pointer to last node of list if there are lead zero, or 0

    %%search_loop:
        ;print2 eax, print_node_ptr
        cmp byte[eax], 0 ; data of curr = 0?
        je %%update_bPtr
        jne %%reset_bPtr
        %%update_bPtr:   ; data = 0
            cmp ebx, 0    ; suspected lead zero?
            jne %%extract_next   ; no need to update bPtr
            mov ebx, ecx ; bPtr = prev   ; new suspected lead zero
            jmp %%extract_next
        %%reset_bPtr:
            mov ebx, 0 ;no lead zero
            jmp %%extract_next
        %%extract_next:
            mov ecx, eax; prev = curr
            inc eax ; get pointer to next node
            mov edx, dword[eax] ; ; get next node
            mov eax, edx ; eax = new curr
            cmp eax,0 ; curr = null?
            jne %%search_loop
            
    %%finish_search:
        ;print1 str
        ;print2 ebx,print_node_ptr
        mov dword[temp],0
        mov dword[temp], ebx ; temp = pointer to next node
        cmp ebx, 0 ; suspected lead zero?
        je %%finish_remove ; no lead zero
        
        inc ebx ; get pointer to first lead zero node
        mov eax, dword[ebx] ; get first lead zero node
       
        ;print2 eax,print_node_ptr
        mov edx ,0                  ;edx = counter
        mov ecx, 0                  ; reset
        %%push_pointers_loop:
            push eax                ;push pointer to stack
            ;print2 eax,print_node_ptr
            inc edx                 ;count++
            mov ecx, eax            ;ecx = pointer to node
            inc ecx                 ;go to next pointer location
            mov eax, dword [ecx]    ;get next pointer

            cmp eax, 0              ;pointer = nullptr?
            jne %%push_pointers_loop    

        %%free_loop:
            pop eax     ;pop node pointer
            dec edx     ;count--

            pushad          ;free node
            pushfd
            push eax
            call free
            add esp, 4
            popfd
            popad

            cmp edx, 0          ;no more pointers to free
            jne %%free_loop

    
    ;print1 str
   ; mov eax, dword [ebx]    ; eax = pointer to node from the last node
    ;print2 dword[temp],print_node_ptr   ; eax = next node
    inc dword[temp]   
    mov eax, 0
    mov eax, dword[temp]                  
    mov dword[eax],0               ;reset the pointer to null
    %%finish_remove:
    popad
%endmacro

insertToStack:
    startFunc
    maxCapacityCheck
    mov ecx, dword [curCapacity]
    cmp ecx,0                       ;if capacity != 0
    je .create_node
    add dword [stackPtr], 4         ;move the stack pointer to the next location
    .create_node:
        createNode ; creates new calloc to lastptr
        insert_node_to_stack
        call inputProcess
        endFunc
        ret  ;return to mycalc

inputProcess:
    startFunc
    mov ecx, buffer 
    .skip_starting_zero:
        cmp byte [ecx], 48
        jne .continue_process
        inc ecx
        jmp .skip_starting_zero

    .continue_process:
        mov edx, 0
        mov dword [len], 0
        
        .length:
            cmp byte[ecx], 0
            je .lenfinish
            cmp byte[ecx], 10
            je .lenfinish
            inc ecx
            inc edx
            jmp .length 
        .lenfinish:
            mov dword[len], edx

            dec ecx    ;pointer to the end of the input
            ;mov dword [numPtr], ecx ;pointer to the end of the input

            .convert_hex:
                
                mov ebx, 0
                mov byte [total], 0
                cmp dword [len],0           ;if len=0 finish
                je .finishInputProcess
                
                readSingleLetter
                
                mov byte [total], al
                dec ecx                     ;numPtr--
                dec dword[len]              ;len--

                cmp dword [len],0 
                je .insertSingle
                jne .insetDouble
                
                
        .insertSingle: ;insert the last digit to the node (the last pointer set to 0 anyway)
        
            mov eax, dword [lastPtr]
            mov bl,byte[total]
            mov byte[eax], bl 
        
            je .finishInputProcess

        .insetDouble:
            
            
            readSingleLetter           ;get the second digit
            
            shl byte al, 4            ; *16
            add byte [total], al      ; +nextDIgit    
            
            dec ecx                     ;numPtr--
            dec dword[len]              ;len--     
            
            mov eax, dword [lastPtr]
            mov bl,byte[total]
            mov byte[eax], bl 
            
            cmp dword [len], 0
            je .finishInputProcess

            .buildNextNode:
                mov eax,0
                mov eax,dword[lastPtr] ; saves current node ptr
                inc eax 
                createNode          ;insert to lastPtr the pointer from calloc 
                mov ebx,0
                mov ebx, dword[lastPtr] ; new pointer from calloc
                mov dword[eax], ebx ;stores new pointer in old pointer

                jmp .convert_hex
        .finishInputProcess:
                inc dword [curCapacity]
                endFunc
                ret             ;return to insert to stack
    

myCalc:
    startFunc
    .mainLoop:
        print2 dword[stackPtr],print_stack_ptr
        print1 prompt
        read        

        cmp byte [buffer],113 ;q
        jne .checkPlus
        call quit
        mov [ebp-4],eax
        endFunc
        ret     ;return to main
        .checkPlus:
        cmp byte [buffer],43  ;+
        jne .checkPop
        call plus
        jmp .mainLoop
        .checkPop:
        cmp byte [buffer],112   ;p
        jne .checkDuplicate
        call popPrint
        jmp .mainLoop
        .checkDuplicate:
        cmp byte [buffer],100   ;d
        jne .checkAnd
        call duplicate
        jmp .mainLoop
        .checkAnd:
        cmp byte [buffer],38    ;&
        jne .checkOr
        call bitAnd
        jmp .mainLoop
        .checkOr:
        cmp byte [buffer],124  ;|
        jne .checkLen
        call bitOr
        jmp .mainLoop
        .checkLen:
        cmp byte [buffer],110   ;n
        jne .processNumber
        call numDigits
        jmp .mainLoop
        
        .processNumber:
        print_debug_input buffer, print_value
        call insertToStack


        jmp .mainLoop

quit:
    startFunc
    freeStack
    mov eax, dword[operandsNum]     ;set return value
    mov [ebp-4], eax                ;store return value 
    endFunc
    ret                             ;rerurn to mycalc
    
popPrint:
    startFunc
    inc dword [operandsNum]
    minCapacityCheck 1        ;capacity < 1 --> ERROR

    .printNumber:

        mov ebx,0                   ;cur digit
        mov eax,0                   ;pointer to the next node 
        mov ecx,0                   
        mov ecx, dword [stackPtr] ;pointer to the stack
        mov dword [numberOfDigitsToPrint],0
        mov eax,dword[ecx]        ;pointer to the node - value of stack pointer   

        .pushNumber_loop:

            mov bl,byte[eax]                ;the data of the node 
            push ebx                        ;push value
            inc dword [numberOfDigitsToPrint]
            inc eax                         ;point to the node's pointer
            mov ecx, dword [eax]            ;get next pointer
            mov eax, ecx                    ;eax = pointer to next node

            cmp eax, 0                     ;pointer = null?
            je .print_first_without_pedding
            jne .pushNumber_loop

        .print_first_without_pedding:
            dec dword [numberOfDigitsToPrint]
            pop edx                 ;get value
            print2 edx, print_hexa_without_pedding      ;print value
            mov edx, 0                      ;reset edx

        .printNumber_loop:

            cmp dword [numberOfDigitsToPrint], 0        ;finish printing?
            je .finish_printing 

            dec dword [numberOfDigitsToPrint]
            pop edx
            print2 edx, print_hexa
            mov edx, 0
            jmp .printNumber_loop

        .finish_printing:
            print1 print_new_line
            dec dword [curCapacity]
            freeList
            cmp dword[curCapacity], 0
            je .finish_pop  ; no more elements on stack- no need to decrease stackPtr
            sub dword [stackPtr],4
            .finish_pop:
            endFunc
            ret

numDigits:
    startFunc
    inc dword [operandsNum]
    minCapacityCheck 1        ;capacity < 1 --> ERROR

    mov ecx, dword [stackPtr] ;pointer to the stack
    mov ebx,dword[ecx]        ;pointer to the node - value of stack pointer  
    mov edx, 0                  ;length
    .len_loop:
        mov eax, ebx            ;pointer to the node
        add edx,2                 ;length=edx

        
        inc eax             ; set pointer to next node
        mov ebx, dword [eax]    ;next node
        cmp ebx, 0              ;
        jne .len_loop

    dec eax
    cmp byte[eax],16
    cmp byte [eax], 15      ;node of 1/2 digits
    ja .skip                ;node of 2 digit
    dec edx                 ;len-- (node of 1)
    .skip:

        
        freeList ;once we have the legnth - no use of the linked list
        
        createNode
        insert_node_to_stack    ;replace the pointer to new pointer
    
        mov ebx, dword [lastPtr] ;pointer
        mov ecx, 256
        mov eax, edx ;length=eax
    
        mov edx, 0
        .insert_length_loop:
            div ecx ;eax=eax/256 , reminder=edx
            mov byte [ebx], dl
            cmp eax, 0
            je .finish_numDigits
            mov dword [temp], eax
            createNode ; lastPtr=pointer from calloc 
            mov eax,dword[lastPtr]
            inc ebx
            mov dword [ebx], eax
            mov eax, dword [temp]
            jmp .insert_length_loop

        .finish_numDigits:    
            mov eax, dword[stackPtr]
            print_debug_result dword[eax]
            endFunc
            ret

duplicate:
    startFunc
    inc dword [operandsNum]
    minCapacityCheck 1        ;capacity < 1 --> ERROR
    maxCapacityCheck
    mov ecx, dword [stackPtr] ;pointer to the stack
    mov ebx,dword[ecx]        ;pointer to the node - value of stack pointer 
    add dword [stackPtr], 4
    createNode                 ;lastPtr = pointer from calloc 
    insert_node_to_stack
    
    mov eax, dword [lastPtr]
    
    .duplicate_loop:
        mov ecx, 0
        mov cl, byte[ebx]
        mov byte[eax], cl
        inc eax
        inc ebx
        cmp dword [ebx], 0
        je .duplicate_finish
        
        mov ecx, 0
        mov ecx, eax

        createNode ;lastPtr = pointer from calloc 

        mov eax, dword [lastPtr]
        mov dword[ecx], eax

        mov ecx, 0
        mov ecx, dword[ebx]
        mov ebx, ecx
        
        jmp .duplicate_loop
    .duplicate_finish:

        inc dword[curCapacity]
        mov eax, dword[stackPtr]
        print_debug_result dword[eax]
        endFunc
        ret

plus:
    startFunc
    inc dword [operandsNum]
    minCapacityCheck 2       ;capacity < 2 --> ERROR

    setPointers ;set op1, op2, result and start_result to the relevant pointers
    mov dword[carry], 0     ;init carry flag

    .plus_loop:
        mov eax, 0
        mov ebx, 0
        mov ecx, 0
        mov edx, 0
        mov dword [temp],0          ;number of active lists 

        cmp dword[op1], 0       ;if not active list 
        je .alReady              ;set al
        mov ecx, dword [op1]
        mov al, byte[ecx]
        .alReady:

        cmp dword[op2], 0       ;set bl
        je .blReady
        mov edx, dword [op2]
        mov bl, byte[edx]
        .blReady:

        add eax, ebx                ;insert to result al+bl+carry
        add eax, dword[carry]
        cmp eax, 255
        jna .reset_carry         ;if carry = 0 
        sub eax, 256            ; else
        mov dword[carry], 1
        jmp .continue_carr
        .reset_carry:
        mov dword[carry], 0
        .continue_carr:
        mov ecx, dword[result]
        mov byte[ecx], al

        cmp dword[op1],0    ;if op1->next is empty, op1=0 else op1=newPtr
        je .op1Ready
        inc dword[op1] 
        mov eax, dword[op1]
        cmp dword[eax], 0
        jne .updateOp1
        mov dword[op1], 0
        .op1Ready:

        cmp dword[op2],0    ;if op2->next is empty, op2=0 else op2=newPtr
        je .op2Ready
        inc dword[op2]
        mov ebx, dword[op2]
        cmp dword[ebx], 0
        jne .updateOp2
        mov dword[op2], 0
        .op2Ready:

        cmp dword[temp], 0  ;if temp=0 => both lists are empty 
        je .isCarry          ;check carry 
        jne .newResultNode   ; if active list create new node 

        .isCarry:
        cmp dword[carry], 0     ;finish if carry=0 & not active list 
        je .plus_finish                  

        .newResultNode:
        createNode                      ;lastPtr=ptr from calloc 
        mov eax, dword[lastPtr]       
        mov ebx, dword[result]          ;ebx=the place to inster lastPtr
        inc ebx
        mov dword[ebx], eax             
        mov dword[result], eax
        jmp .plus_loop

        
    .updateOp1:
        inc dword[temp]
        mov ecx, dword[eax]
        mov dword[op1], ecx
        jmp .op1Ready
    .updateOp2:
        inc dword[temp]
        mov edx, dword[ebx]
        mov dword[op2], edx
        jmp .op2Ready


    .plus_finish:

    mov eax, dword[start_result] ;delete op1 and op2 and insert result to stack
    freeList ; free op1
    sub dword[stackPtr],4
    freeList ; free op2
    mov ebx, dword [stackPtr]
    mov dword[ebx], eax     ; replace op2 with result
    dec dword[curCapacity]  ;capacity --

    print_debug_result dword[start_result]
    

    endFunc
    ret
    


bitAnd:
    startFunc
    inc dword [operandsNum]
    minCapacityCheck 2       ;capacity < 2 --> ERROR

    setPointers ;set op1, op2, result and start_result to the relevant pointers

    .bitAnd_loop:
        mov eax, 0 ;data of op1
        mov ebx, 0 ;data of op2
        mov ecx, 0
        mov edx, 0
        mov dword [temp],0          ;number of active lists 

        mov ecx, dword [op1]
        mov al, byte[ecx]
        .bitAnd_alReady:

        mov edx, dword [op2]
        mov bl, byte[edx]
        .bitAnd_blReady:

        and al, bl                ;insert to result al&bl
        mov ecx, dword[result]
        mov byte[ecx], al

        inc dword[op1]   ;get pointer to op1->next 
        mov eax, dword[op1] ;get op1->next
        cmp dword[eax], 0     ;next=0? finish because all other bits are 0,
        je .bitAnd_finish  
        jne .bitAnd_updateOp1
        .bitAnd_op1Ready:

        inc dword[op2] ;get pointer to op2->next
        mov ebx, dword[op2];get op2->next
        cmp dword[ebx], 0 ; next=0? finish because all other bits are 0,
        je .bitAnd_finish
        jne .bitAnd_updateOp2
        .bitAnd_op2Ready:

        .bitAnd_newResultNode:
        createNode                      ;lastPtr=ptr from calloc 
        mov eax, dword[lastPtr]       
        mov ebx, dword[result]          ;ebx=the place to inster lastPtr
        inc ebx
        mov dword[ebx], eax             
        mov dword[result], eax
        jmp .bitAnd_loop

        
    .bitAnd_updateOp1:
        inc dword[temp]
        mov ecx, dword[eax]
        mov dword[op1], ecx
        jmp .bitAnd_op1Ready
    .bitAnd_updateOp2:
        inc dword[temp]
        mov edx, dword[ebx]
        mov dword[op2], edx
        jmp .bitAnd_op2Ready

    .bitAnd_finish:
        
        mov eax, dword[start_result] ;delete op1 and op2 and insert result to stack
        freeList ; free op1
        sub dword[stackPtr],4
        freeList ; free op2
        mov ebx, dword [stackPtr]
        mov dword[ebx], eax     ; replace op2 with result
        dec dword[curCapacity]  ;capacity --

        print_debug_result dword[start_result]
        removeLeadZero dword[start_result]    ; pointer to first node of result
        
        print_debug_result dword[start_result]
        endFunc
        ret


bitOr:
    startFunc
    inc dword [operandsNum]
    minCapacityCheck 2       ;capacity < 2 --> ERROR

    setPointers ;set op1, op2, result and start_result to the relevant pointers
    
    .bitOr_loop:
        mov eax, 0
        mov ebx, 0
        mov ecx, 0
        mov edx, 0
        mov dword [temp],0          ;number of active lists 

        cmp dword[op1], 0       ;if not active list 
        je .bitOr_alReady              ;set al
        mov ecx, dword [op1]
        mov al, byte[ecx]
        .bitOr_alReady:

        cmp dword[op2], 0       ;set bl
        je .bitOr_blReady
        mov edx, dword [op2]
        mov bl, byte[edx]
        .bitOr_blReady:

        or al, bl                ;insert to result al&bl
        mov ecx, dword[result]
        mov byte[ecx], al

        cmp dword[op1],0    ;if op1->next is empty, op1=0 else op1=newPtr
        je .bitOr_op1Ready
        inc dword[op1] 
        mov eax, dword[op1]
        cmp dword[eax], 0
        jne .bitOr_updateOp1
        mov dword[op1], 0
        .bitOr_op1Ready:

        cmp dword[op2],0    ;if op2->next is empty, op2=0 else op2=newPtr
        je .bitOr_op2Ready
        inc dword[op2]
        mov ebx, dword[op2]
        cmp dword[ebx], 0
        jne .bitOr_updateOp2
        mov dword[op2], 0
        .bitOr_op2Ready:
            cmp dword[temp], 0  ;if temp=0 => both lists are empty 
            je .bitOr_finish   ; if active list create new node 

        .bitOr_newResultNode:
            createNode                      ;lastPtr=ptr from calloc 
            mov eax, dword[lastPtr]       
            mov ebx, dword[result]          ;ebx=the place to inster lastPtr
            inc ebx
            mov dword[ebx], eax             
            mov dword[result], eax
            jmp .bitOr_loop

        
    .bitOr_updateOp1:
        inc dword[temp]
        mov ecx, dword[eax]
        mov dword[op1], ecx
        jmp .bitOr_op1Ready
    .bitOr_updateOp2:
        inc dword[temp]
        mov edx, dword[ebx]
        mov dword[op2], edx
        jmp .bitOr_op2Ready

    .bitOr_finish:
        
        mov eax, dword[start_result] ;delete op1 and op2 and insert result to stack
        
        freeList ; free op1
        sub dword[stackPtr],4
        freeList ; free op2
        mov ebx, dword [stackPtr]
        mov dword[ebx], eax     ; replace op2 with result
        dec dword[curCapacity]  ;capacity --

        print_debug_result dword[start_result]
        endFunc
        
        ret

main:
    getDebug
    getStackSize
    createStack
    print2 dword[stackPtr],print_stack_ptr
    print2 dword[stackHead],print_stack_ptr
    call myCalc
    print2 eax, print_hexa_without_pedding
    print1 print_new_line
    ret
