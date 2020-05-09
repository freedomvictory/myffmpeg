cmake_minimum_required(VERSION 3.13)
project(ffmpeg C)

set(CMAKE_C_STANDARD 99)

add_executable(ffmpeg  decode/decode_hello.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -lavformat -lavcodec -lavutil ")