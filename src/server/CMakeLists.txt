# 定义了一个SRC_LIST遍历，包含了该目录下所有的源文件
aux_source_directory(. SRC_LIST)

aux_source_directory(./database DB_LIST)   #将database下的.cpp也放入
aux_source_directory(./model MODEL_LIST)   #将model下的.cpp也放入
aux_source_directory(./redis REDIS_LIST)   #将redis下的.cpp也放入

# 指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

# 指定可执行文件链接时需要的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)
