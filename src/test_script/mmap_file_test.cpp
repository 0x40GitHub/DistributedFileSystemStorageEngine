#include "mmap_file.h"

using namespace dfse;
using namespace largefile;

const static largefile::MMapOption mmap_option{10240000, 4096, 4096};
int open_file(std::string file_name, int flags)
{
  int fd = open(file_name.c_str(), flags, 0644);
  if (fd < 0)
  {
    // errno
    return -errno;
  }
  return fd;
}
int main()
{
  const char *file_name = "./mapfile_test.txt";
  int fd = open_file(file_name, O_RDWR | O_CREAT | O_LARGEFILE);
  if (fd < 0)
  {
    fprintf(stderr, "open file failed. filename:%s, error desc:%s\n", file_name,
            strerror(-fd));
  }

  largefile::MMapFile *map_file = new largefile::MMapFile(mmap_option, fd);
  bool is_mapped = map_file->map_file(true);
  if (is_mapped)
  {
    map_file->remap_file();
    memset(map_file->get_data(), '9', map_file->get_size());
    map_file->sync_file();
    map_file->unmap_file();
  }
  else
  {
    fprintf(stderr, "map file failed\n");
  }
  close(fd);
  return 0;
}