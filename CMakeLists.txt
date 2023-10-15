cmake_minimum_required(VERSION 3.10)

project(dfse)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# 使用Glob获取源文件
file(GLOB SOURCE_FILES "./src/*.cpp")

include_directories("./include")


# test MMapFile
add_executable(mmap_file "./src/test_script/mmap_file_test.cpp" "./src/mmap_file.cpp")

# test FileOperation
add_executable(file_op "./src/test_script/file_op_test.cpp" "./src/file_op.cpp")

# test MMapFileOperation
add_executable(mmap_file_op "./src/test_script/mmap_file_op_test.cpp" "./src/mmap_file_op.cpp" "./src/file_op.cpp" "./src/mmap_file.cpp")

# test IndexHandle
add_executable(index_handle ./src/test_script/index_handle_test.cpp ${SOURCE_FILES})

# test block write 向块中写入文件
add_executable(block_write ./src/test_script/block_write_test.cpp ${SOURCE_FILES})

# test block read 从块中读取文件
add_executable(block_read ./src/test_script/block_read_test.cpp ${SOURCE_FILES})


# test block delete 从块中删除文件
add_executable(block_del ./src/test_script/block_delete_test.cpp ${SOURCE_FILES})

# test block status 查询块信息
add_executable(block_stat ./src/test_script/block_status.cpp ${SOURCE_FILES})

# test block organization
add_executable(block_org ./src/test_script/main_block_file_organization.cpp ${SOURCE_FILES})

# 清理
# usage: cmake --build . --target clean_files
# 定义要删除的目标文件列表
set(TARGET_FILES
    ${CMAKE_BINARY_DIR}/mmap_file
    ${CMAKE_BINARY_DIR}/file_op
    ${CMAKE_BINARY_DIR}/mmap_file_op
    ${CMAKE_BINARY_DIR}/index_handle
    ${CMAKE_BINARY_DIR}/block_write
    ${CMAKE_BINARY_DIR}/block_read
    ${CMAKE_BINARY_DIR}/block_del
    ${CMAKE_BINARY_DIR}/block_org
    ${CMAKE_BINARY_DIR}/block_stat
    # 添加更多目标文件路径
)
add_custom_target(clean_files
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
     COMMAND ${CMAKE_COMMAND} -E remove ${TARGET_FILES}

    COMMENT "Cleaning build files"
)

