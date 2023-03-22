# include "page_alloc.h"

int main() {
  initial_heap();
  printf("After initialization:\n");
  print_info();

  for(int i = 0; i < 10; i++) {
    pm_malloc(i + 10, 64);
  }
  printf("After calling 10 times of pm_malloc:\n");
  print_info();

  sleep(1);
  pm_malloc(6, 64);
  printf("After calling the 11th time of pm_malloc, because there is out of space in os_heap, there should be one progress moved to disk:\n");
  print_info();

  sleep(1);
  pm_malloc(8, 64);
  printf("After calling the 12th time of pm_malloc, because there is out of space in os_heap, there should be one more progress moved to disk:\n");
  print_info();

  sleep(1);
  pm_run(10);
  printf("The system wants the progress with page number of 10. It calls the pm_run(10) where the progress with page number 10 is in the disk:\n");
  print_info();

  sleep(1);
  pm_free(13);
  printf("After pm_free the data with id 13:\n");
  print_info();

  sleep(1);
  pm_free(11);
  printf("After pm_free the data with id 11 where the progress is in the disk:\n");
  print_info();

  sleep(1);
  pm_malloc(22, 64);
  printf("After calling the 13th time of pm_malloc:\n");
  print_info();

  for(int i = 0; i < 19; i++) {
    sleep(1);
    pm_malloc(i + 30, 64);
    printf("After calling the %dth time of pm_malloc, because there is out of space in os_heap, there should be one more progress moved to disk:\n", i + 14);
    print_info();
  }

  sleep(1);
  printf("When try to insert more pages/progresses into the system now, because no more space in disk and in os_heap, it will fail.\nWhen try to call pm_malloc again, it will show:\n");
  pm_malloc(50, 64);

  sleep(1);
  pm_run(6);
  printf("\nThe system wants the progress with page number of 6 again. It calls the pm_run(6) where the progress with page number 6 is in the disk:\n");
  print_info();

  free_all();

  return 0;
}