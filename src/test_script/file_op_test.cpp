#include "file_op.h"

#include "common.h"
using namespace dfse;
using namespace largefile;

int main()
{
  const char *file_name = "./file_op.txt";
  FileOperation *file_op =
      new FileOperation(file_name, O_CREAT | O_RDWR | O_LARGEFILE);

  int fd = file_op->open_file();
  if (fd < 0)
  {
    fprintf(stderr, "open file %s failed. reason:%s\n", file_name,
            strerror(-fd));
    exit(-1);
  }
  char buffer[64 + 1];
  memset(buffer, '4', sizeof(buffer));

  int ret = file_op->pwrite_file(buffer, sizeof(buffer), 1024);
  if (ret < 0)
  {
    if (ret == largefile::EXIT_DISK_OPER_INCOMPLETE)
    {
      fprintf(stderr, "pwrite file:write length is less than required.");
    }
    else
    {
      fprintf(stderr, "pwrite file %s failed. reason:%s\n", file_name,
              strerror(-ret));
    }
  }

  memset(buffer, '0', sizeof(buffer));
  ret = file_op->pread_file(buffer, 64, 1024);
  if (ret < 0)
  {
    if (ret == largefile::EXIT_DISK_OPER_INCOMPLETE)
    {
      fprintf(stderr, "pread file:read length is less than required.");
    }
    else
    {
      fprintf(stderr, "pread file %s failed. reason:%s\n", file_name,
              strerror(-ret));
    }
  }
  else
  {
    buffer[64] = '\0';
    printf("read:%s\n", buffer);
  }

  file_op->close_file();
}