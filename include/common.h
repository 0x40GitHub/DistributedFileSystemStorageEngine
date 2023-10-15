#ifndef COMMON_H
#define COMMON_H
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#define DEBUG true
namespace dfse
{
  namespace largefile
  {

    const int32_t EXIT_DISK_OPER_INCOMPLETE = -8012;
    const int32_t FS_SUCCESS = 0;
    const int32_t FS_ERROR = -1;
    const int32_t EXIT_INDEX_ALREADY_LOADED_ERROR = -8013;
    const int32_t EXIT_META_UNEXPECT_FOUND_ERROR = -8014;
    const int32_t EXIT_INDEX_CORRUPT_ERROR = -8015; // 索引文件损坏
    const int32_t EXIT_BLOCKID_CONFLICT_ERROR = -8016;
    const int32_t EXIT_BUCKET_CONFIGURE_ERROE = -8017;
    const int32_t EXIT_META_NOT_FOUND_ERROR = -8018;
    const int32_t EXIT_BLOCKID_ZERO_ERROR = -8019;

    enum OperType
    {
      C_OPER_INSERT = 1,
      C_OPER_DELETE = 2
    };
    // 文件映射选项
    struct MMapOption
    {
      uint32_t max_mmap_size_; // 最大可映射的内存大小
      uint32_t first_mmap_size_;
      uint32_t per_mmap_size_;
    };
    static const std::string MAINBLOCK_DIR_PREFIX = "/mainblock/";
    static const std::string INDEX_DIR_PREFIX = "/index/";
    static const mode_t DIR_MODE = 0755;
    // 块信息
    struct BlockInfo
    {
      uint32_t block_id_;      // 块id
      int32_t version_;        // 块当前版本号
      int32_t file_count_;     // 当前已保存的文件数量
      int32_t size_;           // 当前已保存的文件数据的总大小
      int32_t del_file_count_; // 已删除的文件数量
      int32_t del_size_;       // 已删除的文件数据总大小
      uint32_t seq_no_;        // 下一个可分配的文件编号
      BlockInfo() { memset(this, 0, sizeof(BlockInfo)); }

      inline bool operator==(const BlockInfo &rhs) const
      {
        return block_id_ == rhs.block_id_ && version_ == rhs.version_ &&
               file_count_ == rhs.file_count_ && size_ == rhs.size_ &&
               del_file_count_ == rhs.del_file_count_ &&
               del_size_ == rhs.del_size_ && seq_no_ == rhs.seq_no_;
      }
    };

    struct MetaInfo
    {
    public:
      MetaInfo() { init(); }
      MetaInfo(const uint64_t &fileid, const int32_t &inner_offset,
               const int32_t &file_size, const int32_t &next_meta_offset)
      {
        fileid_ = fileid;
        location_.inner_offset_ = inner_offset;
        location_.size_ = file_size;
        next_meta_offset_ = next_meta_offset;
      }
      // 拷贝构造
      MetaInfo(const MetaInfo &meta_info)
      {
        memcpy(this, &meta_info, sizeof(MetaInfo));
      }
      // 拷贝赋值
      MetaInfo &operator=(const MetaInfo &meta_info)
      {
        if (this == &meta_info)
        {
          return *this;
        }
        fileid_ = meta_info.fileid_;
        location_.inner_offset_ = meta_info.location_.inner_offset_;
        location_.size_ = meta_info.location_.size_;
        next_meta_offset_ = meta_info.next_meta_offset_;
      }
      // clone
      MetaInfo &clone(const MetaInfo &meta_info)
      {
        assert(this != &meta_info);
        fileid_ = meta_info.fileid_;
        location_.inner_offset_ = meta_info.location_.inner_offset_;
        location_.size_ = meta_info.location_.size_;
        next_meta_offset_ = meta_info.next_meta_offset_;
        return *this;
      }

      // operator ==
      bool operator==(const MetaInfo &rhs) const
      {
        return fileid_ == rhs.fileid_ &&
               location_.inner_offset_ == rhs.location_.inner_offset_ &&
               location_.size_ == rhs.location_.size_ &&
               next_meta_offset_ == rhs.next_meta_offset_;
      }

      uint64_t get_key() const { return fileid_; }
      void set_key(const uint64_t &key) { fileid_ = key; }
      uint64_t get_fileid() const { return fileid_; }
      void set_fileid(const uint64_t &fileid) { fileid_ = fileid; }
      int32_t get_inner_offset() const { return location_.inner_offset_; }
      void set_inner_offset(const int32_t inner_offset)
      {
        location_.inner_offset_ = inner_offset;
      }
      int32_t get_size() const { return location_.size_; }
      void set_size(const int32_t &size) { location_.size_ = size; }
      int32_t get_next_meta_offset() const { return next_meta_offset_; }
      void set_next_meta_offset(const int32_t &next_meta_offset)
      {
        next_meta_offset_ = next_meta_offset;
      }

    private:
      uint64_t fileid_; // 文件编号
      struct
      {
        int32_t inner_offset_; // 文件在块内部的偏移
        int32_t size_;         // 文件大小
      } location_;
      int32_t next_meta_offset_;

    private:
      void init()
      {
        fileid_ = 0;
        location_.inner_offset_ = 0;
        location_.size_ = 0;
        next_meta_offset_ = 0;
      }
    };

  } // namespace largefile
} // namespace dfse
#endif