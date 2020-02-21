project(mris_pval_fill)

include_directories(${FS_INCLUDE_DIRS})

add_executable(mris_pval_fill mris_pval_fill.cpp)
target_link_libraries(mris_pval_fill utils)

install(TARGETS mris_pval_fill DESTINATION bin)
