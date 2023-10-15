#include "file_op.h"

namespace dfse
{
  namespace largefile
  {

    FileOperation::FileOperation(const std::string &file_name, const int flags)
        : fd_(-1), flags_(flags)
    {
      file_name_ = strdup(file_name.c_str());
    }

    FileOperation::~FileOperation()
    {
      if (fd_ > 0)
      {
        ::close(fd_);
      }
      if (NULL != file_name_)
      {
        free(file_name_);
        file_name_ = NULL;
      }
    }

    int FileOperation::open_file()
    {
      if (fd_ > 0)
      {
        ::close(fd_);
        fd_ = -1;
      }

      fd_ = ::open(file_name_, flags_, OPEN_MODE);
      if (fd_ < 0)
      {
        return -errno;
      }
      return fd_;
    }

    int FileOperation::close_file()
    {
      if (fd_ < 0)
      {
        return 0;
      }
      if (-1 == ::close(fd_))
      {
        return -errno;
      }
      fd_ = -1;
      return 0;
    }

    int64_t FileOperation::get_file_size()
    {
      int fd = check_file();
      if (fd < 0)
      {
        return fd;
      }
      struct stat stat_buf;
      if (fstat(fd, &stat_buf) < 0)
      {
        return -errno;
      }
      return stat_buf.st_size;
    }

    int FileOperation::check_file()
    {
      if (fd_ < 0)
      {
        fd_ = open_file();
      }
      return fd_;
    }

    int FileOperation::ftruncate_file(const uint64_t length)
    {
      int fd = check_file();
      if (fd < 0)
      {
        return fd;
      }
      if (ftruncate(fd, length) == -1)
      {
        return -errno;
      }
      return 0;
    }

    int FileOperation::seek_file(const int64_t offset)
    {
      int fd = check_file();
      if (fd < 0)
      {
        return fd;
      }
      if (lseek(fd, offset, SEEK_SET) == -1)
      {
        return -errno;
      }
      return offset;
    }

    int FileOperation::flush_file()
    {
      if (flags_ & O_SYNC) // 同步写入
      {
        return 0;
      }
      int fd = check_file(); // 检查文件是否打开
      if (fd < 0)
      {
        return fd;
      }
      // 将缓冲区数据写回磁盘
      if (fsync(fd) == -1)
      {
        return -errno;
      }
      return 0;
    }

    int FileOperation::unlink_file()
    {
      int ret = close_file();
      if (ret == -1)
      {
        return ret;
      }
      ret = unlink(file_name_);
      if (ret == -1)
      {
        return -errno;
      }
      return ret;
    }

    int FileOperation::pread_file(char *buf, const uint32_t nbytes,
                                  const int64_t offset)
    {
      uint32_t left = nbytes; // 多次读取
      uint64_t read_offset = offset;
      uint32_t read_len = 0;
      char *p_tmp = buf;
      int i = 0;
      while (left > 0)
      {
        i++;
        if (i > MAX_DISK_TIMES)
        {
          break;
        }
        if (check_file() < 0)
        {
          return -errno;
        }
        read_len = ::pread64(fd_, p_tmp, left, read_offset);
        if (read_len < 0)
        {
          read_len = -errno;
          if (-read_len == EINTR || EAGAIN == -read_len)
          {
            continue;
          }
          else if (EBADF == -read_len)
          {
            fd_ = -1;
            return read_len;
          }
          else
          {
            return read_len;
          }
        }
        else if (0 == read_len)
        {
          break;
        }
        left -= read_len;
        p_tmp += read_len;
        read_offset += read_len;
      }
      if (0 != left)
        return EXIT_DISK_OPER_INCOMPLETE;
      return FS_SUCCESS;
    }

    int FileOperation::pwrite_file(const char *buf, const uint32_t nbytes,
                                   const int64_t offset)
    {
      uint32_t left = nbytes;
      uint64_t write_offset = offset;
      uint32_t writen_len = 0;
      char *p_tmp = const_cast<char *>(buf);
      int i = 0;
      while (left > 0)
      {
        i++;
        if (i > MAX_DISK_TIMES)
        {
          break;
        }
        if (check_file() < 0)
        {
          return -errno;
        }
        writen_len = ::pwrite64(fd_, p_tmp, left, write_offset);
        if (writen_len < 0)
        {
          writen_len = -errno;
          if (-writen_len == EINTR || EAGAIN == -writen_len)
          {
            continue;
          }
          else if (EBADF == -writen_len)
          {
            fd_ = -1;
            continue;
          }
          else
          {
            return writen_len;
          }
        }
        left -= writen_len;
        p_tmp += writen_len;
        write_offset += writen_len;
      }
      if (0 != left)
        return EXIT_DISK_OPER_INCOMPLETE;
      return FS_SUCCESS;
    }
    int FileOperation::write_file(const char *buf, const uint32_t nbytes)
    {
      uint32_t left = nbytes;
      uint32_t writen_len = 0;
      char *p_tmp = const_cast<char *>(buf);
      int i = 0;
      while (left > 0)
      {
        i++;
        if (i > MAX_DISK_TIMES)
        {
          break;
        }
        if (check_file() < 0)
        {
          return -errno;
        }
        writen_len = ::write(fd_, p_tmp, left);
        if (writen_len < 0)
        {
          writen_len = -errno;
          if (-writen_len == EINTR || EAGAIN == -writen_len)
          {
            continue;
          }
          else if (EBADF == -writen_len)
          {
            fd_ = -1;
            continue;
          }
          else
          {
            return writen_len;
          }
        }
        left -= writen_len;
        p_tmp += writen_len;
      }
      if (0 != left)
        return EXIT_DISK_OPER_INCOMPLETE;
      return FS_SUCCESS;
    }

  } // namespace largefile
} // namespace dfse