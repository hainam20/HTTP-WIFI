# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/esp/esp-idf/v5.2.2/esp-idf/components/bootloader/subproject"
  "D:/VScode/Code/Http-Wifi/build/bootloader"
  "D:/VScode/Code/Http-Wifi/build/bootloader-prefix"
  "D:/VScode/Code/Http-Wifi/build/bootloader-prefix/tmp"
  "D:/VScode/Code/Http-Wifi/build/bootloader-prefix/src/bootloader-stamp"
  "D:/VScode/Code/Http-Wifi/build/bootloader-prefix/src"
  "D:/VScode/Code/Http-Wifi/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/VScode/Code/Http-Wifi/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/VScode/Code/Http-Wifi/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
