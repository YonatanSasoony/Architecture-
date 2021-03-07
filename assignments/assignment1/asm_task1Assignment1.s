section .date
    format: db "%d" , 10, 0


section .text
    global assFunc
    extern printf
    extern c_checkValidity


assFunc:
       push ebp                 ; backup ebp register - the last activation frame
       mov ebp, esp             ; make current activation frame
       pushad                   ; backup registers
       pushfd                   ; backup flags
       mov ecx, [ebp+12]        ; get y    
       mov ebx, [ebp+8]         ; get x    
       push ecx                 ; push y as arg for c_checkValidity
       push ebx                 ; push x as arg for c_checkValidity
       call c_checkValidity     ; call function with arguments x,y and return result to eax 
                                ; Returns 1 if x>=y ,otherwise returns 0.
       cmp eax, 0               ; compare the return value of c_checkValidity to 0 
       je add                   ; x>y  so calculate x+y by execute add    
       jne subtract             ; x>=y so calculate x-y by execute subtract
       
       add:
            add ebx,ecx         ; x = x+y
            jmp finish          ; execute finish  

       subtract:    
            sub ebx,ecx         ; x = x-y
            jmp finish          ; execute finish 
        
        finish:                 
            push ebx            ; push the result as arg for printf
            push format         ; push format as arg for printf
            call printf         ; print the result to stdout
            add esp,16          ; return esp to the right point for registers backup
                                ; +16 caz we pushed 4 time 4 byte each time
            popfd               ; backup flags
            popad               ; backup registers
            mov esp, ebp        ; return esp to the address which holds ebp backup
            pop ebp             ; restore activation frame of main()
            RET                 ; return from the function
            
            
            
      

