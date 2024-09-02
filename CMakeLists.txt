cmake_minimum_required(VERSION 3.19)

project(ohmylib_prj VERSION 1.0.0 LANGUAGES C)

cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0053 NEW)

include_directories(./includes/)

add_subdirectory(./lib)