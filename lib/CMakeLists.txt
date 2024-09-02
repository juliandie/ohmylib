cmake_minimum_required(VERSION 3.19)

add_library(ohmylib
    ./cstring.c
    ./dictionary.c
    ./lfsr.c
    ./printlog.c
    ./queue.c
    "sysreg.c"
    ./timestamp.c
 "cvector.c")

set_target_properties(ohmylib PROPERTIES LINKER_LANGUAGE C)