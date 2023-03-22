# include <stdlib.h>
# include <stdio.h>
# include <time.h>
# include <stdbool.h>
# include <unistd.h>
# include <math.h>
# include <string.h>

typedef struct pm_heap {
  // id is the page number of the progress
  int id;
  // store all the page number it uses to store the data
  int addr[1]; 
  struct pm_heap* next;
} pm_heap;

typedef struct page_frame {
  // entry array is to record whether the page is used.
  // -1 means the page is empty, 0 means in used but not the head of the data store, 
  // other values exept for -1 and 0 means the current page is the first page that start storing the data
  long entry[10];
} page_frame;

// to record whether is page of the progress is in the os_heap or in the disk 
typedef struct record {
  int id;
  // to record whether the page of the progress is in the os_heap or in the disk
  // if id is a positive number and status equals to -1 means the page is in the disk
  // status greater than or equals to 0 means the page of the progress is in the os_heap
  int status;
  // record the last used timestamp of the progress
  time_t last_used;
} record;

// initialization of all the data structure including the os_heap
void initial_heap();
// pm_malloc of data with the user-specified id and the data size
// id acts as the page number of the progress
void* pm_malloc(int id, int size);
// write the specific page into os_heap
void pm_write(int id, void* pointer);
// try to get access or use the progress with the page id
void* pm_run(int id);
// swap in the progress from the disk to the os_heap
void swap_in(int id, int idx);
// swap out the least recently used progress from the os_heap to the disk
int swap_out();
// free a progress with specific page id
void pm_free(int id);
// accessing a specific page in os_heap
// the page that is passing in will be in the os_heap
void* pm_access(int id);
// print out the information in os_heap
void print_info();
// free the os_heap
void free_all();