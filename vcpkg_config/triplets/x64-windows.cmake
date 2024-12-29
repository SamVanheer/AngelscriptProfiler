set(VCPKG_TARGET_ARCHITECTURE x64)

include(${CMAKE_CURRENT_LIST_DIR}/x64-common.cmake)

set(VCPKG_C_FLAGS "")
set(VCPKG_CXX_FLAGS "/std:c++23")
