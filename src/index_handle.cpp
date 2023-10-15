#include "index_handle.h"

namespace dfse
{
  namespace largefile
  {

    IndexHandle::IndexHandle(const std::string &base_path,
                             const uint32_t main_block_id)
    {
      // 生成索引文件路径
      std::stringstream tmp_stream;
      tmp_stream << base_path << INDEX_DIR_PREFIX << main_block_id;
      std::string index_path;
      tmp_stream >> index_path;

      mmap_file_op_ =
          new MMapFileOperation(index_path, O_CREAT | O_RDWR | O_LARGEFILE);
      is_load_ = false;
    }

    IndexHandle::~IndexHandle()
    {
      if (mmap_file_op_)
      {
        delete mmap_file_op_;
        mmap_file_op_ = NULL;
      }
    }

    int32_t IndexHandle::create(const uint32_t &logic_block_id, const int32_t &_bucket_size, const MMapOption &map_option)
    {
      int ret;
      if (DEBUG)
        printf(
            "create index, block id:%u, bucket size:%d, max_mmap_size:%d, first mmap size:%d, per mmap size:%d\n",
            logic_block_id, _bucket_size, map_option.max_mmap_size_,
            map_option.first_mmap_size_, map_option.per_mmap_size_);
      if (is_load_)
      {
        return EXIT_INDEX_ALREADY_LOADED_ERROR;
      }
      uint64_t file_size = mmap_file_op_->get_file_size();
      if (file_size < 0)
      {
        return FS_ERROR;
      }
      else if (file_size == 0)
      { // 空文件
        // 初始化索引文件头信息
        IndexHeader i_header;
        i_header.block_info_.block_id_ = logic_block_id;
        i_header.block_info_.seq_no_ = 1;
        i_header.bucket_size_ = _bucket_size;

        i_header.index_file_size_ =
            sizeof(IndexHeader) + _bucket_size * sizeof(int32_t);

        // 写入文件
        char *init_data = new char[i_header.index_file_size_];
        memcpy(init_data, &i_header, sizeof(IndexHeader));
        memset(init_data + sizeof(IndexHeader), 0,
               i_header.index_file_size_ - sizeof(IndexHeader));
        ret = mmap_file_op_->pwrite_file(init_data, i_header.index_file_size_, 0);

        delete init_data;
        init_data = NULL;
        if (ret != FS_SUCCESS)
          return ret;

        ret = mmap_file_op_->flush_file();
        if (ret != FS_SUCCESS)
          return ret;
      }
      else
      { // 文件存在
        return EXIT_META_UNEXPECT_FOUND_ERROR;
      }

      // 映射
      ret = mmap_file_op_->mmap_file(map_option);
      if (ret != FS_SUCCESS)
      {
        return ret;
      }
      is_load_ = true;
      if (DEBUG)
      {
        printf(
            "init block(id:%d) success. data file size:%d, index file size:%d, "
            "bucket size:%d, "
            "free head offset:%d, size:%d, seqno:%d, file count:%d, del size:%d, "
            "del file count:%d, version:%d\n",
            logic_block_id, index_header()->data_file_offset_,
            index_header()->index_file_size_, index_header()->bucket_size_,
            index_header()->free_head_offset_, index_header()->free_head_offset_,
            index_header()->block_info_.seq_no_,
            index_header()->block_info_.file_count_,
            index_header()->block_info_.del_size_,
            index_header()->block_info_.del_file_count_,
            index_header()->block_info_.version_);
      }

      return FS_SUCCESS;
    }

    int32_t IndexHandle::load(const uint32_t &logic_block_id,
                              const int32_t &_bucket_size,
                              const MMapOption &map_option)
    {
      int ret = FS_SUCCESS;
      if (is_load_)
      {
        return EXIT_INDEX_ALREADY_LOADED_ERROR;
      }
      int64_t file_size = mmap_file_op_->get_file_size();
      if (file_size < 0)
      {
        return file_size;
      }
      else if (file_size == 0)
      { // 空文件
        return EXIT_INDEX_CORRUPT_ERROR;
      }
      MMapOption tmp_map_option = map_option;
      if (file_size > tmp_map_option.first_mmap_size_ &&
          file_size <= tmp_map_option.max_mmap_size_)
      {
        tmp_map_option.first_mmap_size_ = file_size;
      }
      // 映射
      ret = mmap_file_op_->mmap_file(tmp_map_option);
      if (ret != FS_SUCCESS)
      {
        return ret;
      }
      // 检查块
      if (0 == bucket_size() || 0 == block_info()->block_id_)
      {
        fprintf(stderr, "Index corrupt error. block id:%u, bucket size:%d\n",
                block_info()->block_id_, _bucket_size);
        return EXIT_INDEX_CORRUPT_ERROR;
      }
      // 检查文件大小
      int32_t index_file_size =
          sizeof(IndexHeader) + sizeof(int32_t) * bucket_size();
      if (file_size < index_file_size)
      {
        fprintf(stderr,
                "Index corrupt error, block id:%u, bucket size:%d, file size:%ld, "
                "index file size%d\n",
                block_info()->block_id_, bucket_size(), file_size, index_file_size);
        return EXIT_INDEX_CORRUPT_ERROR;
      }
      if (logic_block_id != block_info()->block_id_)
      {
        fprintf(stderr, "block id conflict. block id:%u, index block id:%u\n",
                logic_block_id, block_info()->block_id_);
        return EXIT_BLOCKID_CONFLICT_ERROR;
      }
      if (_bucket_size != bucket_size())
      {
        fprintf(stderr,
                "index configure error, old bucket size:%d, new bucket size:%d",
                bucket_size(), _bucket_size);
        return EXIT_BLOCKID_CONFLICT_ERROR;
      }
      is_load_ = true;
      if (DEBUG)
      {
        printf(
            "load block(id:%d) success. data file size:%d, index file size:%d, "
            "bucket size:%d, "
            "free head offset:%d, size:%d, seqno:%d, file count:%d, del size:%d, "
            "del file count:%d, version:%d\n",
            logic_block_id, index_header()->data_file_offset_,
            index_header()->index_file_size_, index_header()->bucket_size_,
            index_header()->free_head_offset_, index_header()->free_head_offset_,
            index_header()->block_info_.seq_no_,
            index_header()->block_info_.file_count_,
            index_header()->block_info_.del_size_,
            index_header()->block_info_.del_file_count_,
            index_header()->block_info_.version_);
      }
      return FS_SUCCESS;
    }

    int32_t IndexHandle::remove(const uint32_t logic_block_id)
    {
      if (is_load_)
      {
        if (logic_block_id != block_info()->block_id_)
        {
          fprintf(stderr, "block id conflict. block id:%d, index block id:%d\n",
                  logic_block_id, block_info()->block_id_);
          return EXIT_BLOCKID_CONFLICT_ERROR;
        }
      }
      // 解除映射
      int ret = mmap_file_op_->munmap_file();
      if (FS_SUCCESS != ret)
      {
        return ret;
      }
      // 删除文件
      ret = mmap_file_op_->unlink_file();
      return ret;
    }

    // 写metainfo
    int32_t IndexHandle::write_segment_meta(const uint64_t key, MetaInfo &meta)
    {
      int32_t current_offset = 0;
      int32_t previous_offset = 0;
      // 判断meta是否存在
      int ret = hash_find(key, current_offset, previous_offset);
      if (FS_SUCCESS == ret)
      { // meta冲突
        return EXIT_META_UNEXPECT_FOUND_ERROR;
      }
      else if (ret != EXIT_META_NOT_FOUND_ERROR)
      {
        return ret;
      }
      // meta不存在
      ret = hash_insert(key, previous_offset, meta);
      return ret;
    }

    int32_t IndexHandle::commit_segment_meta(const uint64_t key, MetaInfo &meta)
    {
      int32_t current_offset = 0;
      int32_t previous_offset = 0;
      // 判断meta是否存在
      int ret = hash_find(key, current_offset, previous_offset);
      if (FS_SUCCESS == ret)
      {
        ret = mmap_file_op_->pwrite_file(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), current_offset);
        if (FS_SUCCESS != ret)
        {
          return ret;
        }
      }
      return ret;
    }

    int32_t IndexHandle::read_segment_meta(const uint64_t key, MetaInfo &meta)
    {
      int32_t current_offset = 0;
      int32_t previous_offset = 0;
      int32_t ret = hash_find(key, current_offset, previous_offset);
      if (FS_SUCCESS == ret)
      { // found
        ret = mmap_file_op_->pread_file(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), current_offset);
        return ret;
      }
      else // not found
      {
        return ret;
      }
    }

    int32_t IndexHandle::read_a_segment_meta(const int32_t offset, MetaInfo &meta)
    {
      return mmap_file_op_->pread_file(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), offset);
    }

    int32_t IndexHandle::delete_segment_meta(const uint64_t key)
    {
      int32_t current_offset = 0;
      int32_t previous_offset = 0;
      int32_t ret = hash_find(key, current_offset, previous_offset);
      if (ret != FS_SUCCESS) // not found
      {
        return ret;
      }

      MetaInfo current_meta;
      // 取得需删除节点的后一个节点
      ret = mmap_file_op_->pread_file(reinterpret_cast<char *>(&current_meta), sizeof(MetaInfo), current_offset);
      if (FS_SUCCESS != ret)
      {
        return ret;
      }
      int32_t next_pos = current_meta.get_next_meta_offset();
      // 若需删除的节点是头结点
      if (previous_offset == 0)
      {
        int32_t slot = static_cast<uint32_t>(key % bucket_size());
        bucket_slot()[slot] = next_pos;
      }
      else
      { // 需删除的节点不是头节点
        // 读取上一个节点
        MetaInfo pre_meta;
        ret = mmap_file_op_->pread_file(reinterpret_cast<char *>(&pre_meta), sizeof(MetaInfo), previous_offset);
        if (FS_SUCCESS != ret)
          return ret;
        pre_meta.set_next_meta_offset(next_pos);
        ret = mmap_file_op_->pwrite_file(reinterpret_cast<char *>(&pre_meta), sizeof(MetaInfo), previous_offset);
        if (FS_SUCCESS != ret)
        {
          return ret;
        }
      }
      // 更新块信息
      update_block_info(C_OPER_DELETE, current_meta.get_size());
      // 将删除的meta节点加入可重用链表(循环链表)
      current_meta.set_next_meta_offset(free_head_offset());
      ret = mmap_file_op_->pwrite_file(reinterpret_cast<char *>(&current_meta), sizeof(MetaInfo), current_offset);
      if (FS_SUCCESS != ret)
      {
        return ret;
      }
      index_header()->free_head_offset_ = current_offset;
      if (DEBUG)
        printf("delete_segment_meta - reuse meta info, current offset:%d\n", current_offset);
      return FS_SUCCESS;
    }

    int32_t IndexHandle::hash_find(const uint64_t key, int32_t &current_offset,
                                   int32_t &previous_offset)
    {
      current_offset = 0;
      previous_offset = 0;
      int ret = FS_SUCCESS;
      MetaInfo meta_info;

      // 确定key存放的桶(slot)的位置,slot中存放的是对应链表头的meta在索引文件中的偏移
      int32_t slot = static_cast<uint32_t>(key) % bucket_size();
      // 链表首节点偏移量
      int32_t pos = bucket_slot()[slot];

      // 遍历链表
      for (; pos != 0;)
      {
        ret = mmap_file_op_->pread_file(reinterpret_cast<char *>(&meta_info),
                                        sizeof(MetaInfo), pos);
        if (FS_SUCCESS != ret)
        {
          return ret;
        }
        if (hash_compare(key, meta_info.get_key()))
        {
          current_offset = pos;
          return FS_SUCCESS;
        }
        previous_offset = pos;
        pos = meta_info.get_next_meta_offset();
      }
      return EXIT_META_NOT_FOUND_ERROR; // not found
    }

    int32_t IndexHandle::hash_insert(const uint64_t key, int32_t previous_offset,
                                     MetaInfo &meta)
    {
      int ret = FS_SUCCESS;
      MetaInfo tmp_meta;
      int32_t current_offset = 0;
      // slot
      int32_t slot = static_cast<uint32_t>(key) % bucket_size();
      // 计算meta info的偏移
      if (free_head_offset() != 0) // 从可重用链表中寻找节点
      {
        ret = mmap_file_op_->pread_file(reinterpret_cast<char *>(&tmp_meta), sizeof(MetaInfo), free_head_offset());
        if (FS_SUCCESS != ret)
        {
          return ret;
        }
        current_offset = index_header()->free_head_offset_;
        index_header()->free_head_offset_ = tmp_meta.get_next_meta_offset();
        if (DEBUG)
          printf("reuse meta info, current offset:%d\n", current_offset);
      }
      else
      {
        current_offset = index_header()->index_file_size_;
        index_header()->index_file_size_ += sizeof(MetaInfo);
      }

      // 写入meta info
      meta.set_next_meta_offset(0);
      ret = mmap_file_op_->pwrite_file(reinterpret_cast<const char *>(&meta),
                                       sizeof(MetaInfo), current_offset);
      if (FS_SUCCESS != ret)
      {
        index_header()->index_file_size_ -= sizeof(MetaInfo); // 回滚
      }
      if (0 != previous_offset)
      { // 知晓插入位置
        ret = mmap_file_op_->pread_file(reinterpret_cast<char *>(&tmp_meta),
                                        sizeof(MetaInfo), previous_offset);
        if (FS_SUCCESS != ret)
        {
          index_header()->index_file_size_ -= sizeof(MetaInfo);
          return ret;
        }
        tmp_meta.set_next_meta_offset(current_offset);
        ret = mmap_file_op_->pwrite_file(reinterpret_cast<const char *>(&tmp_meta),
                                         sizeof(MetaInfo), current_offset);
        if (FS_SUCCESS != ret)
        {
          index_header()->index_file_size_ -= sizeof(MetaInfo); // 回滚
        }
      }
      else
      { // first meta
        bucket_slot()[slot] = current_offset;
      }
      return FS_SUCCESS;
    }

    int32_t IndexHandle::update_block_info(const OperType oper_type,
                                           const uint32_t modify_size)
    {
      if (block_info()->block_id_ == 0)
      {
        return EXIT_BLOCKID_ZERO_ERROR;
      }
      if (oper_type == C_OPER_INSERT) // 写入文件
      {
        ++block_info()->version_;
        ++block_info()->file_count_;
        ++block_info()->seq_no_;
        block_info()->size_ += modify_size;
      }
      else if (oper_type == C_OPER_DELETE) // 删除文件
      {
        ++block_info()->version_;
        --block_info()->file_count_;
        block_info()->size_ -= modify_size;
        ++block_info()->del_file_count_;
        block_info()->del_size_ += modify_size;
      }
      if (DEBUG)
      {
        printf(
            "update block info. block id:%d, seqno:%d, file count:%d, del size:%d, "
            "del file count:%d, version:%d, operation type:%d\n",
            index_header()->block_info_.block_id_,
            index_header()->block_info_.seq_no_,
            index_header()->block_info_.file_count_,
            index_header()->block_info_.del_size_,
            index_header()->block_info_.del_file_count_,
            index_header()->block_info_.version_, oper_type);
      }
      return FS_SUCCESS;
    }

    void IndexHandle::commit_block_data_offset(const int32_t file_size)
    {
      reinterpret_cast<IndexHeader *>(mmap_file_op_->get_map_data())
          ->data_file_offset_ += file_size;
    }

    int32_t IndexHandle::flush()
    {
      int ret = mmap_file_op_->flush_file();
      if (FS_SUCCESS != ret)
      {
        fprintf(stderr, "index flush failed. ret:%d, error desc:%s\n", ret,
                strerror(errno));
      }
      return ret;
    }
    IndexHeader *IndexHandle::index_header() const
    {
      return reinterpret_cast<IndexHeader *>(mmap_file_op_->get_map_data());
    }

    BlockInfo *IndexHandle::block_info() const
    {
      return reinterpret_cast<BlockInfo *>(mmap_file_op_->get_map_data());
    }

    int32_t IndexHandle::bucket_size() const
    {
      return reinterpret_cast<IndexHeader *>(mmap_file_op_->get_map_data())
          ->bucket_size_;
    }

    int32_t *IndexHandle::bucket_slot() const
    {
      return reinterpret_cast<int32_t *>(
          reinterpret_cast<char *>(mmap_file_op_->get_map_data()) +
          sizeof(IndexHeader));
    }
    int32_t IndexHandle::get_block_data_offset() const
    {
      return reinterpret_cast<IndexHeader *>(mmap_file_op_->get_map_data())
          ->data_file_offset_;
    }
    int32_t IndexHandle::free_head_offset() const
    {
      return reinterpret_cast<IndexHeader *>(mmap_file_op_->get_map_data())
          ->free_head_offset_;
    }
  } // namespace largefile
} // namespace dfse