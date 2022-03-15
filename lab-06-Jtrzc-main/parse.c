// PID: 730273827
// I pledge the COMP211 honor code.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parse_objects.h"
#include "parse_functions.h"


#define PAGE_SIZE 4096


int size_of_memmap_array(memmap* maps){
    int size = 0;
    while (maps[size].name != NULL){
        size++;
    }
    return size;
}

void parseline(char* line, memmap* mm){
    char map_name[100], fd[10], dummy[200];

    unsigned long int mem_start, mem_end;
    char r, w, x;

    // Formatted Input - Scanf (Section 7.4 in C book)
    // Read in all desired variables
    sscanf(line, "%lx-%lx %c%c%cp %s %s %s %s",
            &mem_start, &mem_end, &r, &w, &x, dummy, fd, dummy, map_name);


    // Populate memmap struct
    mm->start_addr = mem_start;
    mm->r = r;
    mm->w = w;
    mm->x = x;
    mm->end_addr = mem_end;

    // Have to allocate space to copy the string fields
    char* name = (char *) malloc(100);
    strcpy(name, map_name);
    mm->name = name;

    char* linecpy = (char *) malloc(200);
    strcpy(linecpy, line);
    mm->original_input = linecpy; 

    // STUDENT DEFINED FUNCTIONS
    mm->length = memmap_size(*mm);
}


// Print contents of a memmap struct in a nice way.
void pprint(memmap mm){
    printf("Original text: %s", mm.original_input);
    printf(" Map name: %s\n Start address: %lu\n End address: %lu\n Permissions: %c%c%c\n",
            mm.name, mm.start_addr, mm.end_addr, mm.r, mm.w, mm.x);
    printf("Dependent attributes:\n Section %d\n\n",
            mm.section);
}


struct memmap* populate_mmap(){
    char line[200];
    struct memmap* maps = malloc(200 * sizeof(struct memmap));

    int i = 0;

    unsigned long int num_dummy;
    char char_dummy;
    char string_dummy[200];
    char map_name[200];
    while (fgets(line, 200, stdin) != NULL){
        // determine if the memory mapping has a name 
        // in the last column of the proc maps file
        strcpy(map_name, "unpopulated");
        sscanf(line, "%lx-%lx %c%c%cp %s %s %s %s",
            &num_dummy, &num_dummy, &char_dummy, &char_dummy, &char_dummy, 
            string_dummy, string_dummy, string_dummy, map_name);

        // Only include mappings with names in the final array of structs
        if (strcmp(map_name, "unpopulated") != 0) { 
            parseline(line, &maps[i]);
            i++;
        }
    }

    // STUDENT DEFINED FUNCTIONS
    assign_sections(maps);
    assign_filetypes(maps);

    return maps;
}

unsigned long int memmap_size(memmap mm){
	unsigned long int size = mm.end_addr - mm.start_addr;
	return size;
}

void assign_sections(memmap* maps){
	int size = size_of_memmap_array(maps);
	int heapBool = 0;
	int textBool = 0;
	int roDataBool = 0;
	int dataBool = 0;
	for(int i=0; i<size;i++){
		if(maps[i].r == 'r' && maps[i].w == '-' && maps[i].x == 'x' && textBool == 0){
			maps[i].section = 3;
			textBool = 1;
			continue;
			
		}
		if(maps[i].r == 'r' && maps[i].w == '-' && maps[i].x == '-' && roDataBool == 0){
			maps[i].section = 4;
			roDataBool = 1;
			continue;
			
		}
		if(maps[i].r == 'r' && maps[i].w == 'w' && maps[i].x == '-' && dataBool == 0){
			maps[i].section = 5;
			dataBool = 1;
			continue;
			
		}
		if(strcmp("[heap]", maps[i].name) == 0){
			//heap
			maps[i].section = 2;
			heapBool = 1;
			continue;
			
		}
		if(strcmp("[stack]", maps[i].name) == 0){
			//stack
			maps[i].section = 0;
			heapBool = 0;
			continue;
			
		}
		if(heapBool == 1){
			//sharedlib
			maps[i].section = 1;
			
		}

		
	}
}

void assign_filetypes(memmap* maps){
	int size = size_of_memmap_array(maps);
	for(int i=0; i<size;i++){
		if(maps[i].section == 3){
			maps[i].file_type = 1;
			continue;
		}
		if(maps[i].section == 4){
			maps[i].file_type = 1;
			continue;
		}
		if(maps[i].section == 5){
			maps[i].file_type = 1;
			continue;
		}
		if(maps[i].section == 1){
			maps[i].file_type = 0;
			continue;
		}else{
			maps[i].file_type = 2;
		}
		
	}
}

char* linking_type(memmap* maps){
	int size = size_of_memmap_array(maps);
	int counter = 0;
	assign_sections(maps);

	for(int i=0; i<size;i++){
		if(maps[i].section == 1){
			counter = counter+1;
		}
	}
	if(counter > 2){
		char* type = malloc(7);
		type = "dynamic";
		return type;
	}else{
		char* type = malloc(6);
		type = "static";
		return type;
	}
}

int total_bytes_of_section_type(memmap* maps, enum ProcSection stype){
	int size = size_of_memmap_array(maps);
	int counter = 0;
	for(int i=0; i<size;i++){
		if(maps[i].section == stype){
			counter = counter + memmap_size(maps[i]);
		}
	}
	return counter;
}

int total_bytes_of_file_type(memmap* maps, enum FileType ftype){
	int size = size_of_memmap_array(maps);
	int counter = 0;
	for(int i=0; i<size;i++){
		if(maps[i].file_type == ftype){
			counter = counter + memmap_size(maps[i]);
		}
	}
	return counter;
}

int total_pages_of_section_type(memmap* maps, enum ProcSection stype){
	int size = size_of_memmap_array(maps);
	int counter = total_bytes_of_section_type(maps, stype);
	return counter/4096;
}

int total_pages_of_file_type(memmap* maps, enum FileType ftype){
	int size = size_of_memmap_array(maps);
	int counter = total_bytes_of_file_type(maps, ftype);
	return counter/4096;
}











