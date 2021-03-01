set(CMAKE_CXX_FLAGS_UBSAN
    "-fsanitize=undefined"
    CACHE STRING "" FORCE
    )
set(CMAKE_C_FLAGS_UBSAN
    "-fsanitize=undefined"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_THINLTO
    "-flto=thin"
    CACHE STRING "" FORCE
    )
set(CMAKE_C_FLAGS_THINLTO
    "-flto=thin"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_FULLLTO
    "-flto=full"
    CACHE STRING "" FORCE
    )
set(CMAKE_C_FLAGS_FULLLTO
    "-flto=full"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_ASAN
    "-fsanitize=address"
    CACHE STRING "" FORCE
    )
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_TSAN
    "-fsanitize=thread -O3"
    CACHE STRING "" FORCE
    )

set(CMAKE_C_FLAGS_TSAN
    "-fsanitize=thread -O3"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_MSAN
    "-fsanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls -O3"
    CACHE STRING "" FORCE
    )

set(CMAKE_C_FLAGS_MSAN
    "-fsanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls -O3"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_CFISAN
    "-fsanitize=cfi -flto -fvisibility=hidden"
    CACHE STRING "" FORCE
    )

set(CMAKE_C_FLAGS_CFISAN
    "-fsanitize=cfi -flto -fvisibility=hidden"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_COVERAGE
    "-fprofile-instr-generate -fcoverage-mapping --coverage -g -O0"
    CACHE STRING "" FORCE
    )

set(CMAKE_C_FLAGS_COVERAGE
    "-fprofile-instr-generate -fcoverage-mapping --coverage -g -O0"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_PROFILE
    "-pg"
    CACHE STRING "" FORCE
    )

set(CMAKE_C_FLAGS_PROFILE
    "-pg"
    CACHE STRING "" FORCE
    )

set(CMAKE_EXE_LINKER_FLAGS_PROFILE
    "-pg"
    CACHE STRING "" FORCE
    )

set(CMAKE_CXX_FLAGS_LSAN
    "-fsanitize=leak"
    CACHE STRING "" FORCE
    )
set(CMAKE_C_FLAGS_LSAN
    "-fsanitize=leak"
    CACHE STRING "" FORCE
    )

foreach(X IN LISTS FS_EXP_BUILD_CONFIGURATIONS)
  string(TOUPPER ${X} NAME)
  mark_as_advanced(CMAKE_CXX_FLAGS_${NAME} CMAKE_C_FLAGS_${NAME})
endforeach()
mark_as_advanced(CMAKE_EXE_LINKER_FLAGS_PROFILE)
