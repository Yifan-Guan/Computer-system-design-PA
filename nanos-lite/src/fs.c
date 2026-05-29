#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);


size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode) {
  for (size_t i = 0; i < sizeof(file_table) / sizeof(Finfo); i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      if (!file_table[i].read && !file_table[i].write) {
        file_table[i].read = ramdisk_read;
        file_table[i].write = ramdisk_write; 
      }
      return i;
    }
  }
  panic("cannot find file '%s'", pathname);
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  switch (fd) {
    case FD_STDIN: return 0;
    case FD_STDOUT: case FD_STDERR: return 0;
    default: 
      size_t f_size = file_table[fd].size;
      size_t f_open_offset = file_table[fd].open_offset;
      size_t f_disk_offset = file_table[fd].disk_offset;
      size_t f_remain = f_size - f_open_offset;
      if (len > f_remain) {
        len = f_remain;
      }
      size_t ret = file_table[fd].read(buf, f_disk_offset + f_open_offset, len);
      file_table[fd].open_offset += ret;
      return ret;
  }
  
}

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  switch (fd) {
    case FD_STDIN: return 0;
    case FD_STDOUT: case FD_STDERR: 
      for (size_t i = 0; i < len; i++) {
        putch(((char *)buf)[i]);
      }
      return len;
    default: 
      size_t f_size = file_table[fd].size;
      size_t f_open_offset = file_table[fd].open_offset;
      size_t f_disk_offset = file_table[fd].disk_offset;
      size_t f_remain = f_size - f_open_offset;

      if (len > f_remain) {
        len = f_remain;
      }
      size_t ret = file_table[fd].write(buf, f_disk_offset + f_open_offset, len);
      file_table[fd].open_offset += ret;
      return ret;
  }
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  size_t f_size = file_table[fd].size;
  size_t f_open_offset = file_table[fd].open_offset;

  switch (whence) {
    case SEEK_SET: 
      if (offset < 0 || offset > f_size) {
        panic("invalid offset = %d", offset);
      }
      file_table[fd].open_offset = offset; 
      break;

    case SEEK_CUR: 
      if (f_open_offset + offset < 0 || f_open_offset + offset > f_size) {
        panic("invalid offset = %d", offset);
      }
      file_table[fd].open_offset += offset; 
      break;

    case SEEK_END: 
      if (offset > 0 || f_size + offset < 0) {
        panic("invalid offset = %d", offset);
      }
      file_table[fd].open_offset = f_size + offset; 
      break;

    default: 
      panic("invalid whence = %d", whence);
  }

  return file_table[fd].open_offset;
}

int fs_close(int fd) {
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  file_table[fd].open_offset = 0;
  return 0;
}
