# include "page_alloc.h"
# define HEAP_SIZE 20971520 // 20 * 1024 * 1024
# define BLOCK_SIZE 1048576  // 1024 * 1024

// global variable
static char* os_heap;
page_frame* table;
pm_heap* head;
record* progress_record[30];

// initialization of all the data structures
void initial_heap() {
  os_heap = malloc(HEAP_SIZE);
  // for simplification, the first half of the os_heap is used to store the page table
  // the second half of the os_heap is seperated into 10 pages each of size 1048576.
  table = (page_frame *)os_heap;
  head = (pm_heap *) (os_heap + HEAP_SIZE / 2);
  pm_heap *pointer = head;
  progress_record[0] = (record *) ((char *)(os_heap + HEAP_SIZE / 4));
  
  for(int i = 0; i < 10; i++) {
    table -> entry[i] = -1;
  }

  for(int i = 1; i < 10; i++) {
    pm_heap *curr = (pm_heap*)((os_heap + HEAP_SIZE / 2) + BLOCK_SIZE * i);
    pointer -> next = curr;
    pointer = pointer -> next;
  }

  for(int i = 0; i < 30; i++) {
    progress_record[i] = (record *) ((char *)(os_heap + HEAP_SIZE / 4 + sizeof(record) * i));
    progress_record[i] -> id = 0;
    progress_record[i] -> status = -1;
  }
}

// pm_malloc of data with the user-specified id and the data size
void* pm_malloc(int id, int size) {
  // only positive id are allowed
  if(id <= 0) {
    printf("Id need to be a positive integer\n");
    return NULL;
  }
  // check whether the id exists, if yes, then return directly
  for(int i = 0; i < 30; i++) {
    if(progress_record[i] -> id == id) {
      printf("ID exists, please use another one.\n");
      return NULL;
    }
  }
  // check whethere there is empty page available
  int available = 0;
  for(int i = 0; i < 10; i++) {
    if(table -> entry[i] == -1) {
      available++;
      break;
    }
  }
  if(available < 1) {
    int disk_available = 0;
    for(int i = 0; i < 30; i++) {
      if(progress_record[i] -> id == 0) {
        disk_available = 1;
        break;
      }
    }
    if(disk_available == 0) {
      printf("There is no available space both in main memory and in the disk.\n");
      return NULL;
    }
    swap_out();
  }
  // find out where to put the new progress info in progress_record
  pm_heap* pointer;
  int record_idx = 0;
  for(int i = 0; i < 30; i++) {
    if(progress_record[i] -> id == 0) {
      record_idx = i;
      break;
    }
  }
  // insertion operation
  for(int j = 0; j < 10; j++) {
    if(table -> entry[j] == -1) {
      table -> entry[j] = (long)((os_heap + HEAP_SIZE / 2) + BLOCK_SIZE * j);
      pointer = (pm_heap*)((os_heap + HEAP_SIZE / 2) + BLOCK_SIZE * j);
      pointer -> addr[0] = j;
      pm_write(id, pointer);
      progress_record[record_idx] -> id = id;
      progress_record[record_idx] -> status = j;
      time_t curr_time = time(NULL);
      progress_record[record_idx] -> last_used = (long)curr_time;
      break;
    }
  }
  return pointer;
}

// write the specific page into os_heap
void pm_write(int id, void* pointer) {
  pm_heap* pointer_i = (pm_heap*) pointer;
  pointer_i -> id = id;
  return;
}

// try to request the progress with its page id
void* pm_run(int id) {
  bool in_memory = false;
  for(int i = 0; i < 30; i++) {
    if(progress_record[i] -> id == id) {
      in_memory = true;
      // the progress is in the main memory
      if(progress_record[i] -> status >= 0) {
        time_t curr_time = time(NULL);
        progress_record[i] -> last_used = (long)curr_time;
        return pm_access(id);
      } else {
        int available = -1;
        for(int i = 0; i < 10; i++) {
          if(table -> entry[i] == -1) {
            available = i;
            break;
          }
        }
        if(available == -1) {
          available = swap_out();
        }
        swap_in(id, available);
        return pm_access(id);
      }
    }
  }
  if(!in_memory) {
    printf("The progress in not in memory yet, please retry or use pm_malloc to add the process.\n");
    return NULL;
  }
  return NULL;
}

// swap in the progress with its page id to the specific page frame in the os_heap
void swap_in(int id, int idx) {
  if(table -> entry[idx] != -1) {
    printf("Something's wrong, try again.\n");
    return;
  }
  table -> entry[idx] = (long)((os_heap + HEAP_SIZE / 2) + BLOCK_SIZE * idx);
  pm_heap* pointer = (pm_heap*)((os_heap + HEAP_SIZE / 2) + BLOCK_SIZE * idx);
  pointer -> id = id;
  pointer -> addr[0] = idx;

  for(int i = 0; i < 30; i++) {
    if(progress_record[i] -> id == id) {
      progress_record[i] -> status = idx;
      time_t curr_time = time(NULL);
      progress_record[i] -> last_used = (long)curr_time;
      break;
    }
  }
  int size = (int)((ceil(log10(id))+1)*sizeof(char));
  char filename[size];
  sprintf(filename, "%d", id);
  strcat(filename, ".txt");
  int del = remove(filename);
  if(del) {
    printf("Something is wrong, try again.\n");
  }
}

// swap out the progress that is least recently used, 
// then return the page frame that are available for other progress in os_heap
int swap_out() {
  long used_least = 2147483647;
  int progress = -1;
  for(int i = 0; i < 30; i++) {
    if(progress_record[i] -> status >= 0) {
      long curr = progress_record[i] -> last_used;
      if(curr != 0 && curr < used_least) {
        used_least = curr;
        progress = i;
      }
    }
  }
  int size = (int)((ceil(log10(progress_record[progress] -> id))+1)*sizeof(char));
  char filename[size];
  sprintf(filename, "%d", progress_record[progress] -> id);
  char data[size];
  strcpy(data, filename);
  strcat(filename, ".txt");
  
  FILE *fp;
  fp = fopen(filename, "w");
  if(fp == NULL) {
    printf("Unable to create file.\n");
    exit(EXIT_FAILURE);
  }
 
  fputs(data, fp);
  fclose(fp);

  int progress_id = progress_record[progress] -> id;
  progress_record[progress] -> status = -1;
  int idx;
  pm_heap* pointer;
  for(int i = 0; i < 10; i++) {
    pointer = (pm_heap*)((os_heap + HEAP_SIZE / 2) + BLOCK_SIZE * i);
    if(pointer -> id == progress_id) {
      idx = i;
      table -> entry[i] = -1;
      break;
    }
  }
  return idx;
}

// pm_free the data with the corresponding page id
void pm_free(int id) {
  int exist = -1;
  for(int i = 0; i < 30; i++) {
    if(progress_record[i] -> id == id) {
      exist = 1;
      // in the main memory
      if(progress_record[i] -> status >= 0) {
        int idx = progress_record[i] -> status;
        struct pm_heap *pointer = (pm_heap*)(pm_access(id));
        if(pointer == NULL) {
          printf("Something's wrong, please try again.\n");
        }
        pointer -> id = 0;
        table -> entry[idx] = -1;
        progress_record[i] -> id = 0;
        progress_record[i] -> status = -1;
        progress_record[i] -> last_used = 0;
      // the progress is in the disk
      } else if (progress_record[i] -> status < 0) {
        int size = (int)((ceil(log10(id))+1)*sizeof(char));
        char filename[size];
        sprintf(filename, "%d", id);
        strcat(filename, ".txt");
        int del = remove(filename);
        if(!del) {
          progress_record[i] -> id = 0;
          progress_record[i] -> status = -1;
          progress_record[i] -> last_used = 0;
          printf("The file is deleted successfully.\n");
        } else 
          printf("Something is wrong, try again.\n");
      }
    }
  }
  if(exist == -1) {
    printf("The progress is not exist. Try another progress. \n");
  }
  return;
}

// accessing a specific page in os_heap
void* pm_access(int id) {
  long res = -1;
  struct pm_heap *pointer;
  for(int i = 0; i < 10; i++) {
    if(table -> entry[i] != 0 && table -> entry[i] != -1) {
      pointer =  (pm_heap*)((os_heap + HEAP_SIZE / 2) + BLOCK_SIZE * i);
      if(pointer -> id == id) {
        return pointer;
      }
    }
  }
  printf("The page you are looking for is not in the os_heap.\n");
  return NULL;
}

// print out the info of the page_frame and the blocks
void print_info() {
  printf("The page table: ");
  for(int i = 0; i < 10; i++) {
    printf("%ld -> ", table -> entry[i]);
  }
  printf("end\n");

  printf("Block used situation: ");
  struct pm_heap *pointer = head;
  while(pointer != NULL) {
    printf("id: %d -> ", pointer -> id);
    pointer = pointer -> next;
  }
  printf("end\n");

  printf("Record array: ");
  for(int i = 0; i < 30; i++) {
    printf("id: %d, status: %d, last used time: %ld -> ", progress_record[i] -> id, progress_record[i] -> status, progress_record[i] -> last_used);
  }
  printf("end\n\n");
}

// free the os_heap
void free_all() {
  free(os_heap);
}