#include "mmap_file_op.h"

using namespace dfse;
const static largefile::MMapOption mmap_option{10240000, 4096, 4096};
int main()
{
  const char *file_name = "./mmap_file_op.txt";
  largefile::MMapFileOperation *mmfo =
      new largefile::MMapFileOperation(file_name);

  int fd = mmfo->open_file();
  if (fd < 0)
  {
    fprintf(stderr, "open file %s failed. reson:%s\n", file_name,
            strerror(-fd)); // errno
    exit(-1);
  }
  // 映射
  int ret = mmfo->mmap_file(mmap_option);
  if (ret == largefile::FS_ERROR)
  {
    fprintf(stderr, "mmap_file failed. reason:%s\n", strerror(errno));
    mmfo->close_file();
    exit(-2);
  }
  // 写
  char buffer[128 + 1];
  memset(buffer, '6', sizeof(buffer));
  ret = mmfo->pwrite_file(buffer, sizeof buffer, 8);
  if (ret < 0)
  {
    if (ret == largefile::EXIT_DISK_OPER_INCOMPLETE)
    {
      fprintf(stderr, "pwrite file: write length is less than required.");
    }
    else
    {
      fprintf(stderr, "pwrite file %s failed. reason:%s\n", file_name,
              strerror(-ret));
    }
  }
  // 读
  memset(buffer, '0', sizeof(buffer));
  ret = mmfo->pread_file(buffer, sizeof buffer, 8);
  if (ret < 0)
  {
    if (ret == largefile::EXIT_DISK_OPER_INCOMPLETE)
    {
      fprintf(stderr, "pread file: read length is less than required.");
    }
    else
    {
      fprintf(stderr, "pread file %s failed. reason:%s\n", file_name,
              strerror(-ret));
    }
  }
  else
  {
    buffer[128] = '\0';
    printf("read:%s\n", buffer);
  }

  ret = mmfo->pwrite_file(buffer, sizeof buffer, 4000);
  // flush
  ret = mmfo->flush_file();
  if (ret == largefile::FS_ERROR)
  {
    fprintf(stderr, "flush file failed. reason:%s\n", strerror(errno));
  }
  // 解除映射
  mmfo->munmap_file();
  mmfo->close_file();
  return 0;
}