project(mris_transform)

include_directories(${FS_INCLUDE_DIRS})

add_executable(mris_transform mris_transform.cpp)
add_help(mris_transform mris_transform.help.xml)
target_link_libraries(mris_transform utils)

add_test_script(NAME mris_transform_test SCRIPT test.sh DEPENDS mris_transform)
set_property(TEST mris_transform_test PROPERTY LABELS Expensive Integration)
install(TARGETS mris_transform DESTINATION bin)
