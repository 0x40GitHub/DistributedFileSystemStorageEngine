#include "mmap_file_op.h"

#define DEBUG true
namespace dfse
{
  namespace largefile
  {
    int MMapFileOperation::mmap_file(const MMapOption &mmap_option)
    {
      if (mmap_option.max_mmap_size_ < mmap_option.first_mmap_size_)
      {
        return FS_ERROR;
      }
      if (mmap_option.max_mmap_size_ < 0)
      {
        return FS_ERROR;
      }

      int fd = check_file();
      if (fd < 0)
      {
        fprintf(stderr, "MMapFileOperation::mmap_file - checking file failed!\n");
        return FS_ERROR;
      }
      if (!is_mapped_)
      {
        if (map_file_)
          delete map_file_;
        map_file_ = new MMapFile(mmap_option, fd);
        is_mapped_ = map_file_->map_file(true);
      }
      if (is_mapped_)
        return FS_SUCCESS;
      else
        return FS_ERROR;
    }

    int MMapFileOperation::munmap_file()
    {
      if (is_mapped_ && map_file_ != NULL)
      {
        delete map_file_; // 在析构函数中munmap
        is_mapped_ = false;
      }
      return FS_SUCCESS;
    }
    void *MMapFileOperation::get_map_data() const
    {
      if (is_mapped_)
      {
        return map_file_->get_data();
      }
      else
      {
        return NULL;
      }
    }

    int MMapFileOperation::pread_file(char *buf, const uint32_t size,
                                      const int64_t offset)
    {
      // 内存映射扩容一次
      if (is_mapped_ && (offset + size) > map_file_->get_size())
      {
        if (DEBUG)
          fprintf(stdout,
                  "MMapFileOperation::pread_file, size:%d, offset:%" __PRI64_PREFIX
                  "d, map file size:%d. need remap\n",
                  size, offset, map_file_->get_size());
        map_file_->remap_file();
      }
      // 从内存读取
      if (is_mapped_ && (offset + size) <= map_file_->get_size())
      {
        memcpy(buf, (char *)map_file_->get_data() + offset, size);
        return FS_SUCCESS;
      }

      // 从磁盘读取
      return FileOperation::pread_file(buf, size, offset);
    }

    int MMapFileOperation::pwrite_file(const char *buf, const uint32_t size,
                                       const int64_t offset)
    {
      // 内存映射扩容一次
      if (is_mapped_ && (offset + size) > map_file_->get_size())
      {
        if (DEBUG)
          fprintf(stdout,
                  "MMapFileOperation::pwrite_file, size:%d, offset:%" __PRI64_PREFIX
                  "d, map file size:%d. need remap\n",
                  size, offset, map_file_->get_size());
        map_file_->remap_file();
      }
      // 从内存写入
      if (is_mapped_ && (offset + size) <= map_file_->get_size())
      {
        memcpy((char *)map_file_->get_data() + offset, const_cast<char *>(buf), size);
        return FS_SUCCESS;
      }
      // 直接向磁盘写入
      return FileOperation::pwrite_file(buf, size, offset);
    }

    int MMapFileOperation::flush_file()
    {
      // 将内存中的数据刷写到磁盘上
      if (is_mapped_)
      {
        if (map_file_->sync_file())
        {
          return FS_SUCCESS;
        }
        else
        {
          return FS_ERROR;
        }
      }
      // 函数用于将指定文件描述符对应的文件的数据刷写到磁盘上
      return FileOperation::flush_file();
    }
  } // namespace largefile
} // namespace dfse