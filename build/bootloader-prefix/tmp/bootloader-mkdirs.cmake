# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/localuser/.espressif/v6.0/esp-idf/components/bootloader/subproject"
  "/zpool/localuser/PlatformIO/Projects/NoWireOS-Arduino/build/bootloader"
  "/zpool/localuser/PlatformIO/Projects/NoWireOS-Arduino/build/bootloader-prefix"
  "/zpool/localuser/PlatformIO/Projects/NoWireOS-Arduino/build/bootloader-prefix/tmp"
  "/zpool/localuser/PlatformIO/Projects/NoWireOS-Arduino/build/bootloader-prefix/src/bootloader-stamp"
  "/zpool/localuser/PlatformIO/Projects/NoWireOS-Arduino/build/bootloader-prefix/src"
  "/zpool/localuser/PlatformIO/Projects/NoWireOS-Arduino/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/zpool/localuser/PlatformIO/Projects/NoWireOS-Arduino/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/zpool/localuser/PlatformIO/Projects/NoWireOS-Arduino/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
