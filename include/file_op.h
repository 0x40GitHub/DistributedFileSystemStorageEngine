#ifndef FILE_OP_H
#define FILE_OP_H
#include "common.h"
namespace dfse
{
  namespace largefile
  {
    /**
     * @brief 文件操作类
     */
    class FileOperation
    {
    public:
      /**
       * @brief 构造函数
       * @param file_name 文件名
       * @param flags 文件操作方式和属性 默认:O_RDWR | O_LARGEFILE | O_CREAT 支持超过2GB的文件
       */
      FileOperation(const std::string &file_name, const int flags = O_RDWR | O_LARGEFILE | O_CREAT);
      /**
       * @brief 析构函数
       */
      ~FileOperation();
      /**
       * @brief 打开文件
       * @return 成功打开文件返回正确的文件描述符 重复打开返回-1 失败返回-errno
       */
      int open_file();
      /**
       * @brief 关闭文件
       * @return 文件成功关闭返回0 fd_设置为-1 失败返回-errno
       */
      int close_file();
      /**
       * @brief 将文件写入磁盘
       * @return 写入成功返回0 失败返回-errno
       */
      int flush_file();
      /**
       * @brief 删除文件
       * @return 删除成功返回0 失败返回-errno
       */
      int unlink_file();
      /**
       * @brief 读文件
       * @param buf 存放读取数据的缓冲区
       * @param nbytes 要读取的字节数
       * @param offset 读取的位置
       * @return 读取成功返回0 失败返回-errno
       */
      virtual int pread_file(char *buf, const uint32_t nbytes, const int64_t offset);
      /**
       * @brief 写文件
       * @param buf 要写入的数据的缓冲区
       * @param nbytes 要写入的字节数
       * @param offset 写入位置
       * @return 写入成功返回0 失败返回-errno
       */
      virtual int pwrite_file(const char *buf, const uint32_t nbytes, const int64_t offset);
      /**
       * @brief 从文件指针位置写文件
       * @param buf 要写入的数据的缓冲区
       * @param nbytes 要写入的字节数
       * @return 写入成功返回0 失败返回-errno
       */
      int write_file(const char *buf, const uint32_t nbytes);
      /**
       * @brief 获取文件大小
       * @returnc 成功 返回文件大小 失败 返回-errno
       */
      int64_t get_file_size();
      /**
       * @brief 调整文件大小
       * @param length 指定文件大小
       * @return 成功 返回0 失败 返回-errno
       */
      int ftruncate_file(const uint64_t length);
      /**
       * @brief 移动文件指针
       * @param offset 偏移量
       * @return 成功 返回文件指针相对于文件起始的偏移量 失败 返回-errno
       */
      int seek_file(const int64_t offset);
      /**
       * @brief 获取文件描述符
       * @return fd_
       */
      int get_fd() const { return fd_; }

    protected:
      int fd_;
      int flags_; // 指定文件操作的方式和属性
      char *file_name_;
      static const mode_t OPEN_MODE = 0644; // 文件权限
      static const int MAX_DISK_TIMES = 5;  // 最大磁盘读写次数尝试次数
      /**
       * @brief 检查文件是否打开
       * @return 成功 返回fd_ 失败 返回-erron
       */
      int check_file();
    };

  } // namespace largefile
} // namespace dfse
#endif