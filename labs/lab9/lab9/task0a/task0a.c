#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


int main (int argc, char** argv){

    char* file_name = argv[1];
    int fd = -1 ;
    struct stat fd_stat;  
    void *map_start;
    Elf32_Ehdr *header; 

    if((fd = open(file_name, O_RDONLY)) < 0){
        perror("error in open");
        return 0;
    }

    if(fstat(fd, &fd_stat) != 0 ) {
        if(close(fd) < 0)
            perror("error in close*");
        perror("stat failed");
        return 0;
    }

    if ( (map_start = mmap(0, fd_stat.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED ) {
        if(close(fd) < 0)
            perror("error in close*");
        perror("mmap failed");
        return 0;
    }

    if(!strncmp( ((char*)(map_start+1)),"ELF",3))
        header = (Elf32_Ehdr *) map_start;
    else{
        if(close(fd) < 0)
            perror("error in close*");
        perror("not an ELF");
        munmap(map_start, fd_stat.st_size);
        return 0;
    }

    printf("Type\t\t\tOffset\tVirAddr\t\tPhyAddr\t\tFileSize\tMemSize\tFlag\tAlign\n");
    for(int i=0; i<header->e_phnum; i++){
        Elf32_Phdr* curr = (Elf32_Phdr*) (map_start + header->e_phoff + i * header->e_phentsize);
        printf("%d\t\t\t%#06X\t%#08X\t%#08X\t%#05X\t\t%#05X\t%d\t%#X\n",curr->p_type, curr->p_offset, curr->p_vaddr,curr->p_paddr, curr->p_filesz, curr->p_memsz, curr->p_flags,curr->p_align);
    }
    printf("%d\n", header->e_phentsize);
    return 0;
}

