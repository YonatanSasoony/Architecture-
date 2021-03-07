section	.rodata			; we define (global) read-only variables in .rodata section
	format_string: db "hexa: %s", 10, 0	; format string
	format_decimal: db "decimal: %d", 10, 0	; format string

section .bss			; we define (global) uninitialized variables in .bss section
	an: resb 12		; enough to store integer in [-2,147,483,648 (-2^31) : 2,147,483,647 (2^31-1)]

section .text
	global convertor
	extern printf

convertor:
	push ebp
	mov ebp, esp	
	pushad			

	calc_dec:
	mov ecx, dword [ebp+8]				; get function argument (pointer to string)
	mov eax, 0     						; eax =  total
	mov ebx, 10							; ebx = 10
	mov edx, 0							; edx = curr digit 
	
		calc_dec_loop:
			cmp byte[ecx], 0			; if next char is \0
			je calc_hex			    	; finished - continue to calc to hex
			cmp byte[ecx], 10			; if next char is \n	
			je calc_hex			    	; finished - continue to calc to hex
			mul ebx						; eax = 10*total
			mov dl, byte [ecx]			; dl = curr digit
			sub dl, 48					; get dec by sub ascii		
			add eax, edx		   	 	; eax = eax + dl				
			inc ecx						; ecx++
			jmp calc_dec_loop			; loop 

		
	calc_hex:
			mov ecx,0					; ecx - index
		init_loop:
			mov byte [an+ecx],0
			inc ecx						; ecx++
			cmp ecx,12	
			jne init_loop				; ecx <12

		mov ecx, 16						; ecx = 16   eax =  total
		mov ebx, an						;  ebx = location of current bit

		calc_hex_loop:
			mov edx, 0					; edx = remainder of div
			div ecx				  	    ; eax = eax / 16, edx = eax % 16
			cmp edx, 9	
			ja make_letter       		; remainder > 9
			add edx,48    				; remainder < 9   ==> make to ascii digit
			jmp continue_calc_hex		
			
		make_letter:
			sub edx,10 	 				; get value of letter digit 0-5
			add edx,65 	 				; make to ascii letter
		continue_calc_hex:
			mov [ebx], edx     			; put the remainder in curr location in an
			inc ebx   					; ebx++
			cmp eax,0				
			jne calc_hex_loop			; eax > 0, continue loop

		sub ebx, dword an				; ebx - number of digits
		dec ebx;						; ebx - last index- j
		mov ecx, 0  					; ecx - start index - i
		mov al, 0						; eax - temp1 for an[ebx]
		mov dl, 0						; edx - temp2 for an[ecx]
		
		switch_loop:
			mov al, byte [an+ebx]  		; temp1 = an[i]
			mov dl, byte [an+ecx] 		; temp2 = an[j]
			mov byte [an+ecx], al 		; an[j] = temp1
			mov byte [an+ebx], dl 		; an[i] = temp2
			cmp ebx,ecx					; if i = j, we finished
			je print_hex				; if i = j, we finished
			inc ecx						; ecx++ - i++
			dec ebx						; ecx-- - j--
			cmp ebx,ecx				
			ja switch_loop    		 	; while j>i

	print_hex:
	
		push an					 	; call printf with 2 arguments -  
		push format_string			; pointer to str and pointer to format string
		call printf
		add esp, 8				 	; clean up stack after call

		popad			
		mov esp, ebp	
		pop ebp
		ret
