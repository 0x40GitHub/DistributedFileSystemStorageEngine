#ifndef INDEX_HANDLE_H
#define INDEX_HANDLE_H

#include "common.h"
#include "mmap_file_op.h"
namespace dfse
{
  namespace largefile
  {

    /**
     * @brief 索引文件首部
     */
    struct IndexHeader
    {
    public:
      IndexHeader() { memset(this, 0, sizeof(IndexHeader)); }
      BlockInfo block_info_;     // 块信息
      int32_t bucket_size_;      // 哈希桶的大小
      int32_t data_file_offset_; // 主块文件中未使用数据起始的偏移
      int32_t index_file_size_;  // 索引文件当前偏移(下一个可插入meta的位置)
      int32_t free_head_offset_; // 可重用的链表节点
    };
    /**
     * @brief 索引文件操作类
     */
    class IndexHandle
    {
    public:
      /**
       * @brief 构造函数
       * @param base_path 索引文件基路径 base_path/INDEX_DIR_PREFIX/main_block_id
       * @param main_block_id 主块编号
       */
      IndexHandle(const std::string &base_path, const uint32_t main_block_id);
      ~IndexHandle();

      /**
       * @brief 创建索引文件
       * @param logic_block_id 块编号
       * @param bucket_size 哈希桶大小
       * @param map_option 内存映射选项
       * @return 成功 索引文件被将创建且被映射到内存 失败 返回错误代码
       */
      int32_t create(const uint32_t &logic_block_id, const int32_t &bucket_size, const MMapOption &map_option);

      /**
       * @brief 加载索引文件
       * @param logic_block_id 块编号
       * @param bucket_size 哈希桶大小
       * @param map_option 内存映射选项
       * @return 成功 返回0 失败返回错误代码
       */
      int32_t load(const uint32_t &logic_block_id, const int32_t &bucket_size, const MMapOption &map_option);
      /**
       * @brief 删除索引文件
       * @param logic_block_id
       * @return 成功 返回0 失败返回错误代码
       */
      int32_t remove(const uint32_t logic_block_id);
      /**
       * @brief 写meta info
       * @param key 文件编号
       * @param meta 文件元信息
       * @return 成功 返回0 失败返回错误代码
       */
      int32_t write_segment_meta(const uint64_t key, MetaInfo &meta);
      /**
       * @brief 更新meta info
       * @param key 文件编号
       * @param meta 文件元信息
       * @return 读取成功返回0 失败返回-errno
       */
      int32_t commit_segment_meta(const uint64_t key, MetaInfo &meta);
      /**
       * @brief 读meta info
       * @param key 文件编号
       * @param meta 文件元信息
       * @return 读取成功返回0 失败返回-errno
       */
      int32_t read_segment_meta(const uint64_t key, MetaInfo &meta);
      /**
       * @brief 读一个meta info
       * @param offset 读取位置
       * @param meta 文件元信息
       * @return 读取成功返回0 失败返回-errno
       */
      int32_t read_a_segment_meta(const int32_t offset, MetaInfo &meta);
      /**
       * @brief 删除索引
       * @param key 文件编号
       * @return 读取成功返回0 失败返回-errno
       */
      int32_t delete_segment_meta(const uint64_t key);
      // meta info 哈希查找
      int32_t hash_find(const uint64_t key, int32_t &current_offset,
                        int32_t &previous_offset);
      // meta info哈希插入
      int32_t hash_insert(const uint64_t key, int32_t previous_offset,
                          MetaInfo &meta);
      /**
       * @brief 更新块信息
       * @param oper_type 操作类型 插入/删除
       * @param modify_size 文件大小
       * @return
       */
      int32_t update_block_info(const OperType oper_type, const uint32_t modify_size);
      /**
       * @brief 更新文件数据偏移
       * @param file_size 文件大小
       */
      void commit_block_data_offset(const int32_t file_size);
      /**
       * @brief 将索引文件信息刷新到磁盘
       * @return 成功 返回0 失败 返回-1
       */
      int32_t flush();
      /**
       * Get
       */
      // 得到映射到内存的索引文件头的起始地址
      IndexHeader *index_header() const;
      // 得到块信息起始地址
      BlockInfo *block_info() const;
      // 得到哈希桶的大小
      int32_t bucket_size() const;
      // 得到主块中未使用空间的起始地址
      int32_t get_block_data_offset() const;
      // 返回slot首地址
      int32_t *bucket_slot() const;
      // 获取可重用链表头
      int32_t free_head_offset() const;

    private:
      MMapFileOperation *mmap_file_op_;
      bool is_load_;
      bool hash_compare(const uint64_t l_key, const uint64_t r_key)
      {
        return (l_key == r_key);
      }
    };

  } // namespace largefile
} // namespace dfse
#endif