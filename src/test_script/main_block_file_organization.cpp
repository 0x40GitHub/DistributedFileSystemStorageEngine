#include "file_op.h"
#include "index_handle.h"

using namespace dfse;
// 内存映射选项
const static largefile::MMapOption mmap_option = {1024000, 4096, 4096};
// 哈希桶大小
const static int32_t bucket_size = 1000;
// 主块文件大小
const static uint32_t main_block_size = 1024 * 1024 * 64; // 主块文件大小 64MB
static int32_t block_id = 1;
/**
 * @brief 主块文件整理
 */
int main(int argc, char **argv)
{
    int32_t ret;

    // 输入主块编号
    std::cout << "type your blockid." << std::endl;
    std::cin >> block_id;
    if (block_id < 1)
    {
        std::cerr << "invalid blockid, exit." << std::endl;
        exit(-1);
    }

    // 加载索引文件
    largefile::IndexHandle *index_handle = new largefile::IndexHandle("./src", block_id);
    if (DEBUG)
        printf("init index...\n");
    ret = index_handle->load(block_id, bucket_size, mmap_option);

    if (ret != largefile::FS_SUCCESS)
    {
        fprintf(stderr, "load index %d failed.\n", block_id);
        delete index_handle;
        exit(-2);
    }

    // 异地整理:新建一个主块文件存放未删除的小文件
    // 创建一个新的主块文件
    std::string new_mainblock_path;
    std::stringstream tmp_stream;
    tmp_stream << "./src" << largefile::MAINBLOCK_DIR_PREFIX << INT32_MAX;
    tmp_stream >> new_mainblock_path;
    largefile::FileOperation *new_mainblock = new largefile::FileOperation(
        new_mainblock_path, O_CREAT | O_RDWR | O_LARGEFILE);
    ret = new_mainblock->ftruncate_file(main_block_size);
    if (ret != 0)
    {
        fprintf(stderr, "create main block %s failed. reason:%s\n",
                new_mainblock_path.c_str(), strerror(errno));
        index_handle->remove(block_id);
        delete new_mainblock;
        delete index_handle;
        exit(-2);
    }

    // 打开原主块文件
    std::string mainblock_path;
    tmp_stream.clear();
    tmp_stream << "./src" << largefile::MAINBLOCK_DIR_PREFIX << block_id;
    tmp_stream >> mainblock_path;
    largefile::FileOperation *mainblock = new largefile::FileOperation(mainblock_path, O_RDONLY);

    // 遍历meta info链表,处理每个没有被删除的小文件
    largefile::MetaInfo tmp_meta;
    char *buffer;
    memset(&tmp_meta, 0, sizeof(largefile::MetaInfo));
    int32_t offset;
    int32_t new_offset = 0;
    index_handle->index_header()->data_file_offset_ = 0;
    for (int i = 0; i < index_handle->bucket_size(); i++)
    {
        offset = index_handle->bucket_slot()[i];
        while (offset != 0)
        {
            ret = index_handle->read_a_segment_meta(offset, tmp_meta);
            if (ret != largefile::FS_SUCCESS)
            {
                fprintf(stderr, "read a segment error. offset:%d, reason:%s\n", offset, strerror(errno));
                mainblock->close_file(); // 关闭主块文件
                delete mainblock;
                delete index_handle;
                exit(-2);
            }
            buffer = new char[tmp_meta.get_size()];
            ret = mainblock->pread_file(buffer, tmp_meta.get_size(), tmp_meta.get_inner_offset());
            if (ret != largefile::FS_SUCCESS)
            {
                fprintf(stderr, "read from main block failed. ret:%d, reason:%s\n", ret,
                        strerror(errno));
                mainblock->close_file(); // 关闭主块文件
                delete mainblock;
                delete index_handle;
                exit(-2);
            }
            // 将文件数据写入新的主块
            ret = new_mainblock->pwrite_file(buffer, tmp_meta.get_size(), new_offset);
            if (ret != largefile::FS_SUCCESS)
            {
                fprintf(stderr, "write to main block failed. ret:%d, reason:%s\n", ret,
                        strerror(errno));
                mainblock->close_file();
                delete mainblock;
                delete new_mainblock;
                delete index_handle;
                exit(-3);
            }
            // 更新索引文件信息
            tmp_meta.set_inner_offset(new_offset);
            ret = index_handle->commit_segment_meta(tmp_meta.get_key(), tmp_meta);
            if (ret == largefile::FS_SUCCESS)
            { // 更新块信息
                // 更新索引头信息

                index_handle->commit_block_data_offset(tmp_meta.get_size());
                // 内存数据同步到磁盘
                ret = index_handle->flush();
                if (largefile::FS_SUCCESS != ret)
                {
                    fprintf(stderr, "flush mainblock %d failed. file no:%lu\n", block_id,
                            tmp_meta.get_fileid());
                }
            }
            else
            {
                fprintf(stderr, "move file from mainblock %d failed. fileno:%lu, ret:%d\n",
                        block_id, tmp_meta.get_fileid(), ret);
            }
            if (ret != largefile::FS_SUCCESS)
            {
                fprintf(stderr, "move file from mainblock %d failed. fileno:%lu, ret:%d\n",
                        block_id, tmp_meta.get_fileid(), ret);
            }
            else
            {
                if (DEBUG)
                    printf("move success. file no:%lu, block id:%d\n", tmp_meta.get_fileid(), block_id);
            }

            new_offset += tmp_meta.get_size();
            offset = tmp_meta.get_next_meta_offset();
            delete buffer;
        }
    }
    // 删除原主块文件
    mainblock->unlink_file();
    std::rename(new_mainblock_path.c_str(), mainblock_path.c_str());

    new_mainblock->close_file();
    delete index_handle;
    delete mainblock;
    delete new_mainblock;
    return 0;
}