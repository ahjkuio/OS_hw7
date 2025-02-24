#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// Линейный конгруэнтный генератор
uint64 next(uint64 x) {
	// x(n+1) = (a * x(n) + c) mod m
    // a = 16807 (взято как 7^5, простое число)
    // c = 12345 (простое произвольное небольшое число)
    return x * 16807ULL + 12345ULL;
}

void
test_file(char *name, uint64 size, uint64 seed)
{  
  printf("Testing file %s with size %lu and seed %lu\n", name, size, seed);

  // Создание и запись файла
  printf("\nPhase 1: Creating file and writing data...\n");
  int fd = open(name, O_CREATE | O_RDWR);
  if(fd < 0) {
    printf("Error: Cannot create file\n");
    exit(1);
  }

  uint64 x = seed;
  uint64 written = 0;
  while(written < size) {
    if(written % (size/10) == 0) { // Прогресс каждые 10%
      printf("Writing: %lu%% complete\n", (written * 100) / size);
    }
    if(write(fd, &x, sizeof(x)) != sizeof(x)) {
      printf("Error: Write failed at offset %lu\n", written);
      exit(1);
    }
    written += sizeof(x);
    x = next(x);
  }
  close(fd);
  printf("Write completed. Total bytes written: %lu\n", written);

  // Проверка файла
  printf("\nPhase 2: Verifying file content...\n");
  fd = open(name, O_RDONLY);
  if(fd < 0) {
    printf("Error: Cannot open file for reading\n");
    exit(1);
  }

  x = seed;
  uint64 verified = 0;
  uint64 read_value;
  while(verified < size) {
    if(verified % (size/10) == 0) { // Прогресс каждые 10%
      printf("Verifying: %lu%% complete\n", (verified * 100) / size);
    }
    if(read(fd, &read_value, sizeof(read_value)) != sizeof(read_value)) {
      printf("Error: Read failed at offset %lu\n", verified);
      exit(1);
    }
    if(read_value != x) {
      printf("Error: Verification failed at offset %lu\n", verified);
      printf("Expected: %lu, Got: %lu\n", x, read_value);
      exit(1);
    }
    verified += sizeof(x);
    x = next(x);
  }
  close(fd);
  
  printf("\nTest completed successfully!\n");
  printf("Total bytes verified: %lu\n", verified);
}

int
main(int argc, char *argv[])
{
  if(argc != 4){
    printf("Usage: bigtest filename size seed\n");
    exit(1);
  }

  test_file(argv[1], atoi(argv[2]), atoi(argv[3]));
  exit(0);
}
