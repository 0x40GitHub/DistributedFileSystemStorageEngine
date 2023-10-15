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
    std::string mainblock_path;
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

    // clean
    delete index_handle;
    return 0;
}