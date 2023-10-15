#include "index_handle.h"
#include "file_op.h"

using namespace dfse;
// 内存映射选项
const static largefile::MMapOption mmap_option = {1024000, 4096, 4096};
// 主块文件大小 64MB
const static uint32_t main_block_size = 1024 * 1024 * 64;
// 哈希桶大小
const static int32_t bucket_size = 1000;
// 主块编号
static int32_t block_id = 1;
int main(int argc, char **argv)
{
  // 主块文件路径
  std::string mainblock_path;
  int32_t ret;

  std::cout << "type your blockid." << std::endl;
  std::cin >> block_id;
  if (block_id < 1)
  {
    std::cerr << "invalid blockid, exit." << std::endl;
    exit(-1);
  }

  // 创建索引文件
  largefile::IndexHandle *index_handle = new largefile::IndexHandle("./src", block_id);
  if (DEBUG)
    printf("init index...\n");

  ret = index_handle->create(block_id, bucket_size, mmap_option);
  if (ret != largefile::FS_SUCCESS)
  {
    fprintf(stderr, "create index %d failed.\n", block_id);
    delete index_handle;
    exit(-3);
  }

  // 创建主块文件
  std::stringstream tmp_stream;
  tmp_stream << "./src" << largefile::MAINBLOCK_DIR_PREFIX << block_id;
  tmp_stream >> mainblock_path;

  largefile::FileOperation *mainblock = new largefile::FileOperation(mainblock_path, O_CREAT | O_RDWR | O_LARGEFILE);
  ret = mainblock->ftruncate_file(main_block_size);
  if (ret != 0)
  {
    fprintf(stderr, "create main block %s failed. reason:%s\n", mainblock_path.c_str(), strerror(errno));
    index_handle->remove(block_id);
    delete mainblock;
    exit(-2);
  }

  // 将索引文件刷写到磁盘
  index_handle->flush();

  // 关闭主块文件
  mainblock->close_file();
  delete mainblock;
  delete index_handle;

  return 0;
}