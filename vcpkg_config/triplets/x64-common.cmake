# Included by platform-specific triplets to configure third party dependencies.
# Do not use as a triplet directly.

set(VCPKG_LIBRARY_LINKAGE static)

if(PORT MATCHES "(sdl2)")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
