#include "file_op.h"
#include "index_handle.h"

using namespace dfse;
// 内存映射选项
const static largefile::MMapOption mmap_option = {1024000, 4096, 4096};
// 哈希桶大小
const static int32_t bucket_size = 1000;
static int32_t block_id = 1;
int main(int argc, char **argv)
{
  std::string mainblock_path;
  int32_t ret;
  std::cout << "type your blockid." << std::endl;
  std::cin >> block_id;
  if (block_id < 1)
  {
    std::cerr << "invalid blockid, exit." << std::endl;
    exit(-1);
  }

  // 加载索引文件
  largefile::IndexHandle *index_handle =
      new largefile::IndexHandle("./src", block_id);
  if (DEBUG)
    printf("init index...\n");
  ret = index_handle->load(block_id, bucket_size, mmap_option);

  if (ret != largefile::FS_SUCCESS)
  {
    fprintf(stderr, "load index %d failed.\n", block_id);
    delete index_handle;
    exit(-2);
  }

  // 写入文件到主块文件
  std::stringstream tmp_stream;
  tmp_stream << "./src" << largefile::MAINBLOCK_DIR_PREFIX << block_id;
  tmp_stream >> mainblock_path;

  largefile::FileOperation *mainblock = new largefile::FileOperation(
      mainblock_path, O_CREAT | O_RDWR | O_LARGEFILE);

  char buffer[4096];
  memset(buffer, '6', sizeof(buffer));
  int32_t data_offset = index_handle->get_block_data_offset();

  // 获取可分配的文件编号
  uint32_t file_no = index_handle->block_info()->seq_no_;
  ret = mainblock->pwrite_file(buffer, sizeof(buffer), data_offset);
  if (ret != largefile::FS_SUCCESS)
  {
    fprintf(stderr, "write to main block failed. ret:%d, reason:%s\n", ret,
            strerror(errno));
    mainblock->close_file();
    delete mainblock;
    delete index_handle;
    exit(-3);
  }

  // meta info写入索引文件中
  largefile::MetaInfo meta;
  meta.set_fileid(file_no);
  meta.set_inner_offset(data_offset);
  meta.set_size(sizeof(buffer));
  ret = index_handle->write_segment_meta(meta.get_key(), meta);

  if (ret == largefile::FS_SUCCESS)
  { // 更新块信息
    // 更新索引头信息
    index_handle->commit_block_data_offset(sizeof(buffer));
    // 更新块信息
    index_handle->update_block_info(largefile::C_OPER_INSERT, sizeof(buffer));
    // 内存数据同步到磁盘
    ret = index_handle->flush();
    if (largefile::FS_SUCCESS != ret)
    {
      fprintf(stderr, "flush mainblock %d failed. file no:%u\n", block_id,
              file_no);
    }
  }
  else
  {
    fprintf(stderr, "write segment meta-mainblock %d failed. fileno:%u\n",
            block_id, file_no);
  }
  if (ret != largefile::FS_SUCCESS)
  {
    fprintf(stderr, "write segment meta-mainblock %d failed. fileno:%u\n",
            block_id, file_no);
  }
  else
  {
    if (DEBUG)
      printf("write success. file no:%u, block id:%d\n", file_no, block_id);
  }
  // clean
  mainblock->close_file();
  delete mainblock;
  delete index_handle;

  return 0;
}