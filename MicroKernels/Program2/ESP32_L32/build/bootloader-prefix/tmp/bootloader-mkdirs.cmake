# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.1.1/components/bootloader/subproject"
  "C:/Users/aitor/Desktop/Master_UMA/03_Microkernels/Tema3_MecanismosIPC/L32_RetardosTimersSW/Recursos-20231114/ESP32_L32/ESP32_L32/build/bootloader"
  "C:/Users/aitor/Desktop/Master_UMA/03_Microkernels/Tema3_MecanismosIPC/L32_RetardosTimersSW/Recursos-20231114/ESP32_L32/ESP32_L32/build/bootloader-prefix"
  "C:/Users/aitor/Desktop/Master_UMA/03_Microkernels/Tema3_MecanismosIPC/L32_RetardosTimersSW/Recursos-20231114/ESP32_L32/ESP32_L32/build/bootloader-prefix/tmp"
  "C:/Users/aitor/Desktop/Master_UMA/03_Microkernels/Tema3_MecanismosIPC/L32_RetardosTimersSW/Recursos-20231114/ESP32_L32/ESP32_L32/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/aitor/Desktop/Master_UMA/03_Microkernels/Tema3_MecanismosIPC/L32_RetardosTimersSW/Recursos-20231114/ESP32_L32/ESP32_L32/build/bootloader-prefix/src"
  "C:/Users/aitor/Desktop/Master_UMA/03_Microkernels/Tema3_MecanismosIPC/L32_RetardosTimersSW/Recursos-20231114/ESP32_L32/ESP32_L32/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/aitor/Desktop/Master_UMA/03_Microkernels/Tema3_MecanismosIPC/L32_RetardosTimersSW/Recursos-20231114/ESP32_L32/ESP32_L32/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/aitor/Desktop/Master_UMA/03_Microkernels/Tema3_MecanismosIPC/L32_RetardosTimersSW/Recursos-20231114/ESP32_L32/ESP32_L32/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
