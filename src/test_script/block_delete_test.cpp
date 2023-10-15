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

    // type fileid
    uint64_t fileid = 0;
    std::cout << "type your fileid:" << std::endl;
    std::cin >> fileid;
    if (fileid < 1)
    {
        std::cerr << "invalid fileid." << std::endl;
        exit(-1);
    }

    // 读取meta info
    largefile::MetaInfo meta;
    ret = index_handle->read_segment_meta(fileid, meta);
    if (ret != largefile::FS_SUCCESS)
    {
        fprintf(stderr, "read segment meta error. fileid:%lu, ret:%d\n", fileid, ret);
        exit(-3);
    }
    // 删除指定的文件的meta信息, 没有删除主块文件数据.
    ret = index_handle->delete_segment_meta(fileid);
    if (ret != largefile::FS_SUCCESS)
    {
        fprintf(stderr, "delete index failed. fileid:%lu, ret:%d\n", fileid, ret);
    }
    ret = index_handle->flush();
    if (ret != largefile::FS_SUCCESS)
    {
        fprintf(stderr, "flush mainblock %d failed. file no:%lu\n", block_id, fileid);
    }
    printf("delete index success.\n");

    delete index_handle;
    return 0;
}