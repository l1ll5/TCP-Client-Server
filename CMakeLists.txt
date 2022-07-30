cmake_minimum_required(VERSION 3.14.7)
project(TCP)

set(CMAKE_CXX_STANDARD 17)


aux_source_directory(src/Utils Utils)
aux_source_directory(src/Buffer Buffer)
aux_source_directory(src/EventLoop EventLoop)
aux_source_directory(src/Timer Timer)
aux_source_directory(src/Server/Server Server)
aux_source_directory(src/Server/ThreadPool ThreadPool)
aux_source_directory(src/Connection Connection)
aux_source_directory(src/Epoll Epoll)

aux_source_directory(src/Client Client)

# add_compile_options(-Wall -Wextra)

Include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.9.2
)
FetchContent_MakeAvailable(spdlog)

add_executable(Server test/ServerMain.cpp ${Server} ${ThreadPool} ${EventLoop} ${Epoll} ${Connection} ${Utils} ${Timer} ${Buffer})
target_link_libraries(Server PRIVATE spdlog::spdlog)
add_executable(Client test/ClientMain.cpp ${Client} ${EventLoop} ${Connection} ${Epoll} ${Utils} ${Timer} ${Buffer})
target_link_libraries(Client PRIVATE spdlog::spdlog)

