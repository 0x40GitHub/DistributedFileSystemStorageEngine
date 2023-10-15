#ifndef MMAP_FILE_OP_H
#define MMAP_FILE_OP_H
#include "common.h"
#include "file_op.h"
#include "mmap_file.h"
namespace dfse
{
  namespace largefile
  {
    /**
     * @brief 文件映射操作类
     */
    class MMapFileOperation : public FileOperation
    {
    public:
      /**
       * @brief 构造函数
       * @param file_name 文件名
       * @param flags 文件操作标志和属性
       */
      MMapFileOperation(const std::string file_name, const int flags = O_RDWR | O_LARGEFILE | O_CREAT)
          : FileOperation(file_name, flags), map_file_(NULL), is_mapped_(false) {}
      ~MMapFileOperation()
      {
        if (map_file_)
        {
          delete map_file_;
          map_file_ = NULL;
        }
      }
      /**
       * @brief 从内存读取
       * @param buf 存放读取数据的缓冲区
       * @param size 读取的字节数
       * @param offset 读取位置
       * @return 读取成功返回0 失败返回-errno
       */
      virtual int pread_file(char *buf, const uint32_t size, const int64_t offset) override;
      /**
       * @brief 向内存写入
       * @param buf 要写入数据的缓冲区
       * @param size 写入的字节数
       * @param offset 写入位置
       * @return 写入成功返回0 失败返回-errno
       */
      virtual int pwrite_file(const char *buf, const uint32_t size, const int64_t offset) override;
      /**
       * @brief 映射
       * @param mmap_option 映射选项
       * @return 读取成功返回0 失败返回-1
       */
      int mmap_file(const MMapOption &mmap_option);
      /**
       * @brief 取消映射
       * @return 成功 返回0
       */
      int munmap_file();
      /**
       * @brief 获取文件映射首地址
       * @return 返回文件映射首地址
       */
      void *get_map_data() const;
      /**
       * @brief 将内存映射文件刷写到磁盘
       * @return 成功 返回0 失败 返回-1
       */
      int flush_file();

    private:
      MMapFile *map_file_;
      bool is_mapped_;
    };
  } // namespace largefile
} // namespace dfse
#endif