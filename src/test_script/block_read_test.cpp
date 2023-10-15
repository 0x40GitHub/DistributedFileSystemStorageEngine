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

    // type fileid
    uint64_t fileid = 0;
    std::cout << "type your fileid:" << std::endl;
    std::cin >> fileid;
    if (fileid < 1)
    {
        std::cerr << "invalid fileid." << std::endl;
        exit(-1);
    }

    // read meta info
    largefile::MetaInfo meta;
    ret = index_handle->read_segment_meta(fileid, meta);
    if (ret != largefile::FS_SUCCESS)
    {
        fprintf(stderr, "read segment meta error. fileid:%lu, ret:%d\n", fileid, ret);
        exit(-3);
    }
    // read file by meta info
    std::stringstream tmp_stream;
    tmp_stream << "./src" << largefile::MAINBLOCK_DIR_PREFIX << block_id;
    tmp_stream >> mainblock_path;

    largefile::FileOperation *mainblock = new largefile::FileOperation(
        mainblock_path, O_RDONLY);
    char *buffer = new char[meta.get_size() + 1];
    ret = mainblock->pread_file(buffer, meta.get_size(), meta.get_inner_offset());
    if (ret != largefile::FS_SUCCESS)
    {
        fprintf(stderr, "read from main block failed. ret:%d, reason:%s\n", ret,
                strerror(errno));
        mainblock->close_file(); // 关闭主块文件
        delete mainblock;
        delete index_handle;
        exit(-3);
    }
    // read success
    buffer[meta.get_size()] = '\0';
    printf("read file success:read_len:%ld, %s\n", strlen(buffer), buffer);
    // clean
    delete mainblock;
    delete index_handle;
    return 0;
}