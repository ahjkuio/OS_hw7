#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"

#define BLOCK_SIZE 1024
#define VALUES_PER_BLOCK (BLOCK_SIZE / sizeof(uint64))

// Линейный конгруэнтный генератор
uint64 next(uint64 x) {
	// x(n+1) = (a * x(n) + c) mod m
    // a = 16807 (взято как 7^5, простое число)
    // c = 12345 (простое произвольное небольшое число)
    return x * 16807ULL + 12345ULL;
}

// Возвращает размер файла в байтах
uint64 get_file_size(char *filename) {
    struct stat st;
    if (stat(filename, &st) < 0) {
        return 0;
    }
    return st.size;
}

void
test_file(char *name, uint64 size, uint64 seed)
{  
  uint64 maxsize = MAXFILE * BLOCK_SIZE;
  
  if (size > maxsize) {
    printf("Warning: Size %lu exceeds maximum file size (%lu). Adjusting...\n", size, maxsize);
    size = maxsize;
  }
  
  printf("Testing file %s with size %lu bytes and seed %lu\n", name, size, seed);
  printf("Maximum file size: %lu bytes\n", maxsize);

  // Создание и запись файла
  printf("\nPhase 1: Creating file and writing data...\n");
  int fd = open(name, O_CREATE | O_RDWR);
  if(fd < 0) {
    printf("Error: Cannot create file\n");
    exit(1);
  }

  uint64 buffer[VALUES_PER_BLOCK]; // Буфер для записи блоками
  uint64 x = seed;
  uint64 written = 0;
  uint64 last_progress = 0;
  
  while(written < size) {
    // Показываем прогресс каждые 5% или 10 МБ
    uint64 progress_percent = (written * 100) / size;
    if (progress_percent >= last_progress + 5 || written == 0) {
      printf("Writing: %lu KB (%lu%% complete)\n", written / 1024, progress_percent);
      last_progress = progress_percent;
    }
    
    // Определяем размер текущего блока
    uint64 block_bytes = BLOCK_SIZE;
    if (written + block_bytes > size) {
      block_bytes = size - written;
    }
    uint64 values_to_write = block_bytes / sizeof(uint64);
    
    if (values_to_write == 0) break; // Предотвращаем запись пустых блоков
    
    // Заполняем буфер значениями
    for (uint64 i = 0; i < values_to_write; i++) {
      buffer[i] = x;
      x = next(x);
    }
    
    // Записываем блок
    int bytes_written = write(fd, buffer, values_to_write * sizeof(uint64));
    if(bytes_written != values_to_write * sizeof(uint64)) {
      printf("Error: Write failed at offset %lu. Expected %lu bytes, wrote %d bytes\n", 
             written, values_to_write * sizeof(uint64), bytes_written);
      close(fd);
      exit(1);
    }
    written += bytes_written;
  }
  close(fd);
  printf("Write completed. Total bytes written: %lu KB\n", written / 1024);

  // Проверяем реальный размер файла
  uint64 actual_size = get_file_size(name);
  if (actual_size != written) {
    printf("Warning: File size mismatch. Expected: %lu, Actual: %lu\n", written, actual_size);
  }

  // Проверка файла
  printf("\nPhase 2: Verifying file content...\n");
  fd = open(name, O_RDONLY);
  if(fd < 0) {
    printf("Error: Cannot open file for reading\n");
    exit(1);
  }

  x = seed;
  uint64 verified = 0;
  last_progress = 0;
  
  while(verified < written) {
    // Показываем прогресс каждые 5% или 10 МБ
    uint64 progress_percent = (verified * 100) / written;
    if (progress_percent >= last_progress + 5 || verified == 0) {
      printf("Verifying: %lu KB (%lu%% complete)\n", verified / 1024, progress_percent);
      last_progress = progress_percent;
    }
    
    // Определяем размер текущего блока
    uint64 block_bytes = BLOCK_SIZE;
    if (verified + block_bytes > written) {
      block_bytes = written - verified;
    }
    uint64 values_to_read = block_bytes / sizeof(uint64);
    
    if (values_to_read == 0) break; // Предотвращаем чтение пустых блоков
    
    // Читаем блок
    int bytes_read = read(fd, buffer, values_to_read * sizeof(uint64));
    if(bytes_read != values_to_read * sizeof(uint64)) {
      printf("Error: Read failed at offset %lu. Expected %lu bytes, read %d bytes\n", 
             verified, values_to_read * sizeof(uint64), bytes_read);
      close(fd);
      exit(1);
    }
    
    // Проверяем значения в буфере
    for (uint64 i = 0; i < values_to_read; i++) {
      if(buffer[i] != x) {
        printf("Error: Verification failed at offset %lu\n", verified + i * sizeof(uint64));
        printf("Expected: %lu, Got: %lu\n", x, buffer[i]);
        close(fd);
        exit(1);
      }
      x = next(x);
    }
    
    verified += bytes_read;
  }
  close(fd);
  
  printf("\nTest completed successfully!\n");
  printf("Total bytes verified: %lu KB\n", verified / 1024);
}

int
main(int argc, char *argv[])
{
  if(argc != 4){
    printf("Usage: bigtest filename size seed\n");
    printf("Example: bigtest test1 1048576 12345\n");
    printf("For maximum file size test: bigtest bigfile 67108864 12345\n");
    exit(1);
  }

  // Проверяем корректность значений размера и seed
  uint64 size = atoi(argv[2]);
  uint64 seed = atoi(argv[3]);
  
  // Размер должен быть кратен sizeof(uint64)
  if (size % sizeof(uint64) != 0) {
    // Округляем до кратного sizeof(uint64)
    size = (size / sizeof(uint64)) * sizeof(uint64);
    printf("Warning: Size adjusted to %lu to ensure proper alignment\n", size);
  }
  
  if (size == 0) {
    printf("Error: Size must be greater than 0\n");
    exit(1);
  }
  
  test_file(argv[1], size, seed);
  exit(0);
}
