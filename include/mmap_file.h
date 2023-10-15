#ifndef MMAP_FILE_H
#define MMAP_FILE_H
#include "common.h"
namespace dfse
{
  namespace largefile
  {
    /**
     * @brief 文件映射类
     */
    class MMapFile
    {
    public:
      MMapFile();
      explicit MMapFile(const int fd);
      MMapFile(const MMapOption &mmap_option, const int fd);
      ~MMapFile();
      /**
       * @brief 将内存数据同步到磁盘
       * @return 成功 返回true 失败返回false
       */
      bool sync_file();
      /**
       * @brief 将文件映射到内存
       * @param write 可写标志
       * @return 成功 返回true 失败 返回false
       */
      bool map_file(const bool write = false);
      /**
       * @brief 获取文件内存映射的首地址
       * @return data_ 返回文件内存映射的首地址
       */
      void *get_data() const;
      /**
       * @brief 获取映射文件的大小
       * @return size_ 映射成功的文件大小
       */
      uint32_t get_size() const;
      /**
       * @brief 解除映射
       * @return 成功 返回true 失败 返回false
       */
      bool unmap_file();
      /**
       * @brief 重新映射,根据映射选项追加内存映射空间
       * @return 成功 返回true 失败 返回false
       */
      bool remap_file();

    private:
      /**
       * @brief 调整文件大小
       * @param size 要调整到的大小
       * @return
       */
      bool ensure_file_size(const uint32_t size);
      uint32_t size_;
      int fd_;
      void *data_;
      // 文件映射选项
      struct MMapOption mmap_file_opiton_;
    };

  } // namespace largefile
} // namespace dfse

#endif