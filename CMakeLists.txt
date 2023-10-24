cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_COMPILER "clang-11")
set(CMAKE_CXX_COMPILER "clang++-11")
set(FETCHCONTENT_QUIET FALSE)

project(
  wdissector
  LANGUAGES CXX C
  VERSION 1.0)
include(Modules.cmake)
include(FetchContent)
include(ExternalProject)
set(WIRESHARK_VERSION "3.7.0")
set(WIRESHARK_BIN_DIR "${PROJECT_SOURCE_DIR}/bin")
log(INFO "Current Generator: ${CMAKE_GENERATOR}")
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS
    1
    CACHE INTERNAL "No dev warnings")
# Wireshark source path
set(WS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/libs/wireshark)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${WS_SOURCE_DIR}/cmake/modules
                      ${PROJECT_SOURCE_DIR}/libs/cef-cmake/cmake)
# ---------------- Dependencies --------------------
find_program(ZSTD_EXIST "zstd" REQUIRED)
find_program(FUSER_EXIST "fuser" REQUIRED)
find_program(SSHPASS_EXIST "sshpass" REQUIRED)
find_program(SWIG_EXIST "swig")
find_program(SWIG_EXIST "ctypesgen")
# * pthreads
find_package(Threads REQUIRED)
# * glib-2.0
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
if(GLIB_FOUND)
  log(INFO "Found GLIB-2.0 library")
  log(STATUS "LD Flags: ${GLIB_LDFLAGS}")
  log(STATUS "C Flags: ${GLIB_CFLAGS}")
endif()
# * freetype2
find_package(Freetype REQUIRED)
if(FREETYPE_FOUND)
  log(INFO "Found FreeType2 ${FREETYPE_VERSION_STRING} library")
  log(STATUS "LD Flags: ${FREETYPE_LDFLAGS}")
  log(STATUS "C Flags: ${FREETYPE_CFLAGS}")
endif()
# * Wireshark
find_library(
  LIBWIRESHARK
  NAMES wireshark
  HINTS "${CMAKE_INSTALL_PREFIX}/lib"
  ONLY_CMAKE_FIND_ROOT_PATH NO_DEFAULT_PATH)
find_library(
  LIBWIRESHARK_DEV
  NAMES wireshark
  HINTS "${WIRESHARK_BIN_DIR}"
  ONLY_CMAKE_FIND_ROOT_PATH NO_DEFAULT_PATH)
# * zlib
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  log(INFO "Found ZLIB library")
  log(STATUS "${ZLIB_LIBRARY}")
endif()

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++2a" COMPILER_SUPPORTS_CXX20)
if(NOT COMPILER_SUPPORTS_CXX20)
  message(FATAL_ERROR "Compiler ${CC} does not support C++20")
endif()

# ---------------- Configuration --------------------
# Options
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
set(DEV
    OFF
    CACHE BOOL "Compile this Wireshark library")
set(WIRESHARK_GUI
    OFF
    CACHE BOOL "Compile Wireshark GUI")
set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
    CACHE BOOL "Export compile_commands.json" FORCE)
set(ENABLE_EXPORTS ON)
option(FORCE_COLORED_OUTPUT
       "Always produce ANSI-colored output (GNU/Clang only)." TRUE)

if(${FORCE_COLORED_OUTPUT})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  endif()
endif()
# Configure Output folders
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Global compiler options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(COMMON_FLAGS
    "-fno-stack-protector -fpic -Wall -Wno-braced-scalar-init -Wno-unused-variable -Wno-reorder"
)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${COMMON_FLAGS})
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${COMMON_FLAGS})
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld-11")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld-11")
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_BUILD_RPATH $ORIGIN bin modules/python/install/lib/)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
# Print some options
log(INFO "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
log(INFO "EXPORT_COMPILE_COMMANDS: ${EXPORT_COMPILE_COMMANDS}")

# ---------------- Includes --------------------
# Global Includes
include_directories(
  AFTER ${PROJECT_SOURCE_DIR}/ ${PROJECT_SOURCE_DIR}/src/
  ${PROJECT_SOURCE_DIR}/libs/ ${PROJECT_SOURCE_DIR}/libs/libpcap/
  libs/libvifaced/include)

# WDissector Includes
set(WDISSECTOR_INCLUDES
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/libs/
    ${GLIB_INCLUDE_DIRS}
    ${WS_SOURCE_DIR}/
    ${WS_SOURCE_DIR}/epan
    ${WS_SOURCE_DIR}/epan/ftypes
    libs/serialport/include/
    libs/PcapPlusPlus/Common++/header/
    libs/PcapPlusPlus/Packet++/header/
    libs/PcapPlusPlus/Pcap++/header/)
  

# ---------------- Libraries --------------------
# libPCAP
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/libpcap)
set(HAVE_LIBPCAP 1)
set(ENABLE_PCAP ON)
set(CAN_SET_CAPTURE_BUFFER_SIZE 1)
set(PCAP_FOUND ON)

# GLFW3 (No Wayland)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_USE_OSMESA OFF)
set(GLFW_USE_WAYLAND OFF)
set(BUILD_SHARED_LIBS ON)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/glfw)
add_subdirectory("libs/libvifaced/")

# Configure Pagmo2
log(INFO "Building libpagmo.so")
# 1) Download and preapre liboost headers
set(LIBBOOST_DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/libs/libboost-serialization/)
set(LIBBOOST_DOWNLOAD_URL https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.gz)
string(REGEX MATCH "boost_[^.]*" LIBBOOST_VERSION "${LIBBOOST_DOWNLOAD_URL}")
if(NOT EXISTS ${LIBBOOST_DOWNLOAD_DIR}/boost)
  FetchContent_Declare(
    Boost
    URL ${LIBBOOST_DOWNLOAD_URL}
    DOWNLOAD_DIR ${LIBBOOST_DOWNLOAD_DIR}
    SOURCE_DIR ${LIBBOOST_DOWNLOAD_DIR}
    DOWNLOAD_NAME "boost.tar.gz"
    UPDATE_COMMAND echo "Extracting boost.tar.gz. Please wait..." && tar -xf boost.tar.gz && ln -sf ${LIBBOOST_VERSION}/boost boost
    CONFIGURE_COMMAND ""
    DOWNLOAD_NO_EXTRACT TRUE
  )
  FetchContent_MakeAvailable(Boost)
endif()
# 2) Import libboost shared libraries
set(LIBBOOST_SERIALIZATION libboost_serialization.so.1.78.0)
set(LIBBOOST_IOSTREAMS libboost_iostreams.so.1.78.0)
set(Boost_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libs/libboost-serialization/boost_1_78_0/)

add_library(libboost_serialization SHARED IMPORTED)
add_library(Boost::serialization ALIAS libboost_serialization)
set_target_properties(
  libboost_serialization PROPERTIES IMPORTED_LOCATION
                            ${PROJECT_SOURCE_DIR}/libs/libboost-serialization/${LIBBOOST_SERIALIZATION})
target_include_directories(libboost_serialization INTERFACE ${PROJECT_SOURCE_DIR}/libs/libboost-serialization/boost_1_78_0)

add_library(libboost_iostreams SHARED IMPORTED)
add_library(Boost::boost ALIAS libboost_iostreams)
set_target_properties(
  libboost_iostreams PROPERTIES IMPORTED_LOCATION
                            ${PROJECT_SOURCE_DIR}/libs/libboost-serialization/${LIBBOOST_IOSTREAMS})
                            target_include_directories(libboost_iostreams INTERFACE ${PROJECT_SOURCE_DIR}/libs/libboost-serialization/boost_1_78_0)

# 3) Copy libboost shared libraries to bin folder
add_custom_command(
  OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIBBOOST_SERIALIZATION}
  COMMAND cp libs/libboost-serialization/${LIBBOOST_SERIALIZATION}
          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIBBOOST_SERIALIZATION}
  DEPENDS libs/libboost-serialization/${LIBBOOST_SERIALIZATION}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
add_custom_target(
  copy_libboost-serialization
  DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIBBOOST_SERIALIZATION})

add_custom_command(
  OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIBBOOST_IOSTREAMS}
  COMMAND cp libs/libboost-serialization/${LIBBOOST_IOSTREAMS}
          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIBBOOST_IOSTREAMS}
  DEPENDS libs/libboost-serialization/${LIBBOOST_IOSTREAMS}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
add_custom_target(
  copy_libboost-iostreams
  DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIBBOOST_IOSTREAMS})

# 4) Add subdirectories and link libboost to pagmo              
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/pagmo2)
add_dependencies(pagmo copy_libboost-serialization copy_libboost-iostreams)
target_link_libraries(pagmo PUBLIC libboost_iostreams)
include_directories(${PROJECT_SOURCE_DIR}/libs/pagmo2/include)

# Chrome Embbeded Framework (CEF) library
set(CEF_USE_SANDBOX
OFF
CACHE BOOL "Force turning off of sandbox")
include(cef_cmake)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/cef-cmake)

# Cross-platform serial library
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/serialport)

# Graphviz C++ Library (gvpp target)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/gvpp)

# tinyprocess (process library)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/tinyprocess)

# react-cpp (event loop library)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/react-cpp/)

# Pcap PlusPlus
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/PcapPlusPlus/)
set(PcapPlusPlus CommonPP PacketPP PcapPP)
include_directories(
  libs/PcapPlusPlus/Pcap++/header/
  libs/PcapPlusPlus/Packet++/header/
  libs/PcapPlusPlus/Common++/header/)

# RTAudio
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/rtaudio/)

# PyBind11 (Run Python Modules from C++)
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/modules/python/success)
  file(
    DOWNLOAD
    https://github.com/indygreg/python-build-standalone/releases/download/20201020/cpython-3.8.6-x86_64-unknown-linux-gnu-pgo-20201020T0627.tar.zst
    ${PROJECT_SOURCE_DIR}/modules/python.tar.zst
    SHOW_PROGRESS)
  execute_process(
    COMMAND bash -c "tar -I zstd -xf python.tar.zst && rm python.tar.zst"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/modules/
    ERROR_VARIABLE PYTHON_ERROR)
  if(PYTHON_ERROR)
    message(FATAL_ERROR ${PYTHON_ERROR})
  endif(PYTHON_ERROR)
  # Install requirements from modules/server/requirements.txt
  execute_process(
    COMMAND bash -c
            "./python/install/bin/python3 -m pip install -r ./requirements.txt"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/modules/
    ERROR_VARIABLE PYTHON_ERROR)
  if(PYTHON_ERROR)
    message(FATAL_ERROR ${PYTHON_ERROR})
  endif(PYTHON_ERROR)
  # Save success indication file
  execute_process(
    COMMAND bash -c "touch python/success"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/modules/
    ERROR_VARIABLE PYTHON_ERROR)
  if(PYTHON_ERROR)
    message(FATAL_ERROR ${PYTHON_ERROR})
  endif(PYTHON_ERROR)
endif()
set(PYTHON_EXECUTABLE ${PROJECT_SOURCE_DIR}/modules/python/install/bin/python3)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/pybind11/)
target_link_directories(pybind11::embed INTERFACE
                        ${PROJECT_SOURCE_DIR}/modules/python/install/lib/)

# Dynamo (shared library loader)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/dynalo/)

# Import and build libzmq
add_subdirectory(libs/libzmq)

# Bluetooth Stack
set(BTSTACK_ROOT ${PROJECT_SOURCE_DIR}/3rd-party/btstack/)
add_subdirectory(${BTSTACK_ROOT})
set(BT_INCLUDES
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/libs
    ${BTSTACK_ROOT}/3rd-party/micro-ecc
    ${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/include
    ${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/include
    ${BTSTACK_ROOT}/3rd-party/md5
    ${BTSTACK_ROOT}/3rd-party/hxcmod-player
    ${BTSTACK_ROOT}/3rd-party/hxcmod-player/mod
    ${BTSTACK_ROOT}/3rd-party/lwip/core/src/include
    ${BTSTACK_ROOT}/3rd-party/lwip/dhcp-server
    ${BTSTACK_ROOT}/3rd-party/rijndael
    ${BTSTACK_ROOT}/3rd-party/yxml
    ${BTSTACK_ROOT}/3rd-party/tinydir
    ${BTSTACK_ROOT}/chipset/bcm
    ${BTSTACK_ROOT}/chipset/csr
    ${BTSTACK_ROOT}/src
    ${BTSTACK_ROOT}/platform/posix
    ${BTSTACK_ROOT}/platform/embedded
    ${BTSTACK_ROOT}/platform/lwip
    ${BTSTACK_ROOT}/platform/lwip/port
    ${BTSTACK_ROOT})

# Build Wireshark if needed
if(NOT WIRESHARK_GUI)
  # Disable Wireshark GUI by default
  set(ENABLE_LUA
      OFF
      CACHE BOOL "")
  set(ENABLE_STATIC
      OFF
      CACHE BOOL "")
  set(BUILD_wireshark
      OFF
      CACHE BOOL "")
  set(BUILD_fuzzshark
      OFF
      CACHE BOOL "")
  set(BUILD_androiddump
      OFF
      CACHE BOOL "")
  set(BUILD_capinfos
      OFF
      CACHE BOOL "")
  set(BUILD_captype
      OFF
      CACHE BOOL "")
  set(BUILD_ciscodump
      OFF
      CACHE BOOL "")
  set(BUILD_dcerpcidl2wrs
      OFF
      CACHE BOOL "")
  set(BUILD_dftest
      OFF
      CACHE BOOL "")
  set(BUILD_dpauxmon
      OFF
      CACHE BOOL "")
  set(BUILD_dumpcap
      OFF
      CACHE BOOL "")
  set(BUILD_editcap
      OFF
      CACHE BOOL "")
  set(BUILD_mergecap
      OFF
      CACHE BOOL "")
  set(BUILD_mmdbresolve
      OFF
      CACHE BOOL "")
  set(BUILD_randpkt
      OFF
      CACHE BOOL "")
  set(BUILD_randpktdump
      OFF
      CACHE BOOL "")
  set(BUILD_rawshark
      OFF
      CACHE BOOL "")
  set(BUILD_reordercap
      OFF
      CACHE BOOL "")
  set(BUILD_sdjournal
      OFF
      CACHE BOOL "")
  set(BUILD_sharkd
      OFF
      CACHE BOOL "")
  set(BUILD_sshdump
      OFF
      CACHE BOOL "")
  set(BUILD_text2pcap
      OFF
      CACHE BOOL "")
  set(BUILD_tfshark
      OFF
      CACHE BOOL "")
  set(BUILD_tshark
      OFF
      CACHE BOOL "")
  set(BUILD_udpdump
      OFF
      CACHE BOOL "")
else()
  set(ENABLE_LUA
      OFF
      CACHE BOOL "")
  set(ENABLE_STATIC
      OFF
      CACHE BOOL "")
endif()

if(LIBWIRESHARK)
  log(INFO "Found global Wireshark library")
  log(STATUS "${LIBWIRESHARK}")
endif()

if(LIBWIRESHARK_DEV)
  log(INFO "Found dev. Wireshark library, using it for builds")
  log(STATUS "${LIBWIRESHARK_DEV}")
endif()

if(LIBWIRESHARK_DEV)
  log(INFO "Building dev. Wireshark library")
  add_subdirectory(${WS_SOURCE_DIR})
  add_library(wireshark ALIAS epan)

elseif(LIBWIRESHARK AND DEV)
  log(INFO "Building dev. Wireshark library")
  add_subdirectory(${WS_SOURCE_DIR})
  add_library(wireshark ALIAS epan)
  set(LIBWIRESHARK_DEV ON)
elseif(NOT LIBWIRESHARK)
  log(WARNING "libwireshark.so was not found, building wireshark...")
  add_subdirectory(${WS_SOURCE_DIR})
  add_library(wireshark ALIAS epan)
  set(LIBWIRESHARK_DEV ON)
endif()

# Build oai_tracing folder
add_library(
  tracing STATIC
  libs/oai_tracing/database.c libs/oai_tracing/utils.c
  libs/oai_tracing/handler.c libs/oai_tracing/event.c libs/oai_tracing/config.c)
target_compile_options(tracing PRIVATE "-w")

# Build wireapi.a
add_library(
  wireapi STATIC
  libs/whereami.c
  libs/profiling.c
  libs/logs.c
  ${WS_SOURCE_DIR}/ui/cmdarg_err.c
  ${WS_SOURCE_DIR}/ui/clopts_common.c
  ${WS_SOURCE_DIR}/ui/util.c
  ${WS_SOURCE_DIR}/ui/dissect_opts.c
  ${WS_SOURCE_DIR}/ui/failure_message.c
  ${WS_SOURCE_DIR}/ui/iface_toolbar.c
  ${WS_SOURCE_DIR}/ui/decode_as_utils.c
  ${WS_SOURCE_DIR}/ui/version_info.c
  ${WS_SOURCE_DIR}/frame_tvbuff.c
  ${WS_SOURCE_DIR}/capture/capture-pcap-util.c
  ${WS_SOURCE_DIR}/capture/capture-pcap-util-unix.c
  ${WS_SOURCE_DIR}/cfile.c
  ${WS_SOURCE_DIR}/file.c
  ${WS_SOURCE_DIR}/file_packet_provider.c
  ${WS_SOURCE_DIR}/extcap.c
  ${WS_SOURCE_DIR}/extcap_parser.c
  ${WS_SOURCE_DIR}/wsutil/please_report_bug.c)
target_compile_options(wireapi PRIVATE ${GLIB_CFLAGS} -w)
target_include_directories(wireapi PUBLIC ${WDISSECTOR_INCLUDES})
target_link_libraries(
  wireapi
  PUBLIC ${GLIB_LDFLAGS}
         wireshark
         wiretap
         wsutil
         pcap
         -lgmodule-2.0
         -lz
         -lm
         -lpcre2-8)

if(LIBWIRESHARK_DEV)
  log(INFO "Added plugins dependency")
  add_dependencies(wireapi plugins)
endif()

# Build libwdissector.a
add_library(libwdissector STATIC src/wdissector.cpp)
target_link_libraries(libwdissector PUBLIC wireapi)
target_compile_options(libwdissector PRIVATE "-DWDISSECTOR_STATIC_LIB")
set_target_properties(libwdissector PROPERTIES CXX_VISIBILITY_PRESET default
                                               VISIBILITY_INLINES_HIDDEN 0)

# Build libwdissector.so
add_library(wdissector SHARED src/wdissector.cpp)
target_link_libraries(wdissector PUBLIC wireapi)
target_compile_options(wdissector PRIVATE "-DWDISSECTOR_STATIC_LIB")
set_target_properties(wdissector PROPERTIES CXX_VISIBILITY_PRESET default
                                            VISIBILITY_INLINES_HIDDEN 0)

# Build imgui
add_library(
  imgui STATIC
  libs/imgui/imgui_impl_glfw.cpp
  libs/imgui/imgui_impl_opengl3.cpp
  libs/imgui/libs/gl3w/GL/gl3w.c
  libs/imgui/imgui.cpp
  libs/imgui/imgui_draw.cpp
  libs/imgui/imguial_term.cpp
  libs/imgui/imgui_widgets.cpp
  libs/imgui/misc/freetype/imgui_freetype.cpp
  libs/imgui/misc/fonts/DroidSans.cpp
  libs/imgui/misc/fonts/DroidSansMono.cpp
  libs/imgui/misc/fonts/IconsFontAwesome5.cpp
  libs/imgui/ImGuiFileDialog/ImGuiFileDialog.cpp)
target_compile_options(imgui PRIVATE -w)
target_compile_definitions(imgui PRIVATE -DIMGUI_IMPL_OPENGL_LOADER_GL3W)
target_include_directories(
  imgui
  PUBLIC libs/imgui/libs/gl3w/ libs/imgui/ ${FREETYPE_INCLUDE_DIRS}
         ${PROJECT_SOURCE_DIR}/ ${PROJECT_SOURCE_DIR}/libs libs/glfw/include)
target_link_libraries(imgui PRIVATE GL glfw rt m dl ${FREETYPE_LIBRARIES})

add_library(nanolog STATIC libs/NanoLog.cpp)
target_include_directories(nanolog PRIVATE ${PROJECT_SOURCE_DIR}/)

# Add libmpsse-static (FTDI MPSSI - SPI mode)
add_library(mpsse-static STATIC IMPORTED)
set_target_properties(
  mpsse-static PROPERTIES IMPORTED_LOCATION
                          ${PROJECT_SOURCE_DIR}/libs/libmpsse/libmpsse-static.a)
target_link_libraries(mpsse-static INTERFACE pthread udev)
target_include_directories(mpsse-static INTERFACE libs/libmpsse/)

# ---------------- Targets --------------------
# Targets are compiled with shared libraries associated with wireapi.a
# automatically

# example_wdissector
add_executable(example_wdissector src/wdissector.cpp)
target_link_libraries(example_wdissector PRIVATE wireapi)

# macpdu2wireshark
add_executable(macpdu2wireshark src/macpdu2wireshark.c)
target_link_libraries(macpdu2wireshark PRIVATE tracing libwdissector pthread)
target_compile_options(macpdu2wireshark PRIVATE "-w")

# Common Fuzzer libs
set(FUZZER_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    wdissector
    pagmo
    imgui
    gvc # From graphviz
    cgraph # From graphviz
    libzmq # ZMQ
    gvpp
    tinyprocess
    nanolog
    reactcpp
    ${PcapPlusPlus}
    pybind11::embed
    cefdll_wrapper
    serial
    rtaudio)

# Common Fuzzer libs
set(MINIMAL_FUZZER_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    wdissector
    imgui
    pagmo
    gvc # From graphviz
    cgraph # From graphviz
    libzmq # ZMQ
    gvpp
    tinyprocess
    nanolog
    reactcpp
    ${PcapPlusPlus}
    pybind11::embed
    serial
    rtaudio)

# Common Fuzzer libs
set(EXPLOITER_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    wdissector
    imgui
    libzmq # ZMQ
    gvpp
    tinyprocess
    nanolog
    reactcpp
    ${PcapPlusPlus}
    serial
    rtaudio)

# ------- Fuzzers -------

# LTEFuzzer files
set(LTE_FUZZER_SRC src/lte_fuzzer.cpp libs/shared_memory.c libs/profiling.c)
# LTEFuzzer executable
add_executable(lte_fuzzer ${LTE_FUZZER_SRC})
target_link_libraries(lte_fuzzer PRIVATE ${FUZZER_LIBS})
# target_compile_options(lte_fuzzer PRIVATE -w -flto=thin)
target_compile_options(lte_fuzzer PRIVATE -w)
target_compile_definitions(lte_fuzzer PRIVATE -DFUZZ_LTE)
# add_dependencies(lte_fuzzer quicktype_enb)
target_precompile_headers(lte_fuzzer PRIVATE src/PCHLTE.hpp)

# BTFuzzer files
set(BT_FUZZER_SRC src/bt_fuzzer.cpp libs/profiling.c)
# BTFuzzer executable
add_executable(bt_fuzzer ${BT_FUZZER_SRC})
target_link_libraries(bt_fuzzer PRIVATE ${FUZZER_LIBS})
target_compile_options(bt_fuzzer PRIVATE -w -O3)
target_compile_definitions(bt_fuzzer PRIVATE -DFUZZ_BT)
# add_dependencies(bt_fuzzer quicktype_bt)
target_precompile_headers(bt_fuzzer PRIVATE src/PCHBT.hpp)

# BT Host Fuzzer files
set(BTHOST_FUZZER_SRC src/bthost_fuzzer.cpp libs/profiling.c)
# BT Host Fuzzer executable
add_executable(bthost_fuzzer ${BTHOST_FUZZER_SRC})
target_link_libraries(bthost_fuzzer PRIVATE ${FUZZER_LIBS})
target_compile_options(bthost_fuzzer PRIVATE -w -O0)
target_compile_definitions(bthost_fuzzer PRIVATE -DFUZZ_BTHOST)
# add_dependencies(bt_fuzzer quicktype_bthost)
target_precompile_headers(bthost_fuzzer PRIVATE src/PCHBTHOST.hpp)

# WiFi AP files
set(WIFI_AP_FUZZER_SRC src/wifi_ap_fuzzer.cpp libs/profiling.c)
# WiFi AP executable
add_executable(wifi_ap_fuzzer ${WIFI_AP_FUZZER_SRC})
target_link_libraries(wifi_ap_fuzzer PRIVATE ${MINIMAL_FUZZER_LIBS})
target_compile_options(wifi_ap_fuzzer PRIVATE -w -O0)
target_compile_definitions(wifi_ap_fuzzer PRIVATE -DFUZZ_WIFI_AP)
# add_dependencies(bt_fuzzer quicktype_wifi_ap)
target_precompile_headers(wifi_ap_fuzzer PRIVATE src/PCHWIFI.hpp)

# ------- Exploiters -------

# BTExploiter executable
add_executable(bt_exploiter ${BT_FUZZER_SRC})
target_compile_definitions(bt_exploiter PUBLIC BUILD_EXPLOITER)
target_link_libraries(bt_exploiter PRIVATE ${EXPLOITER_LIBS})
target_compile_options(bt_exploiter PRIVATE "-w" -g3)
target_compile_definitions(bt_exploiter PRIVATE -DFUZZ_BT)

# BT Host Exploiter executable
add_executable(bthost_exploiter ${BTHOST_FUZZER_SRC})
target_compile_definitions(bthost_exploiter PUBLIC BUILD_EXPLOITER)
target_link_libraries(bthost_exploiter PRIVATE ${EXPLOITER_LIBS})
target_compile_options(bthost_exploiter PRIVATE -w -O0)
target_compile_definitions(bthost_exploiter PRIVATE -DFUZZ_BTHOST)

# SHM Test
add_executable(shm_server libs/shm_server.c libs/shared_memory.c)
add_executable(shm_client libs/shm_client.c libs/shared_memory.c)
target_link_libraries(shm_server pthread)
target_link_libraries(shm_client pthread)

# BT programs executable
set(BT_PROGRAMS
    ${PROJECT_SOURCE_DIR}/src/programs/bluetooth/spp_counter.c
    ${PROJECT_SOURCE_DIR}/src/programs/bluetooth/sdp_rfcomm_query.c
    ${PROJECT_SOURCE_DIR}/src/programs/bluetooth/sdp_general_query.c
    ${PROJECT_SOURCE_DIR}/src/programs/bluetooth/sdp_bnep_query.c
    ${PROJECT_SOURCE_DIR}/src/programs/bluetooth/a2dp_sink_demo.c
    ${PROJECT_SOURCE_DIR}/src/programs/bluetooth/a2dp_source_demo.c
    ${PROJECT_SOURCE_DIR}/src/programs/ble/gatt_browser.c)
foreach(BT_PROGRAM_FILE ${BT_PROGRAMS})
  # add main.cpp and run loop
  get_filename_component(BT_PROGRAM ${BT_PROGRAM_FILE} NAME_WE)
  log(INFO "BT Program: ${BT_PROGRAM_FILE}")
  list(APPEND BT_PROGRAM_FILE
       ${BTSTACK_ROOT}/platform/posix/btstack_run_loop_posix.c
       ${PROJECT_SOURCE_DIR}/src/programs/bluetooth/bt_stack.cpp)
  add_executable(${BT_PROGRAM} ${BT_PROGRAM_FILE})
  target_link_libraries(${BT_PROGRAM} btstack-lib pthread)
  target_include_directories(${BT_PROGRAM} PRIVATE ${BT_INCLUDES})
endforeach(EXAMPLE_FILE)

# WDMapper
add_executable(wdmapper src/WDMapper.cpp)
target_link_libraries(wdmapper PRIVATE libwdissector gvc cgraph gvpp
                                       ${PcapPlusPlus} reactcpp)
target_compile_options(wdmapper PRIVATE -w)
# add_dependencies(wdmapper quicktype_generic)
target_precompile_headers(wdmapper PRIVATE src/WDMapper.h)

# ---------------- Custom Commands --------------------
# Generate eNB config class (eNBConfig.hpp)
add_custom_target(
  quicktype_enb
  COMMAND
    quicktype ../configs/enb_config.json -l cpp -o configs/eNBConfig.hpp
    --no-boost --code-format with-struct --top-level MachineConfig &&
    ../scripts/gen_default_config.sh ../configs/enb_config.json
    configs/eNBConfig.hpp >> configs/eNBConfig.hpp
  COMMENT "Generating src/eNBConfig.hpp from configs/enb_config.json"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/)
# Custom target to avoid double build add_custom_target(quicktype_enb DEPENDS
# ${PROJECT_SOURCE_DIR}/src/eNBConfig.hpp)

# Generate BT config class (BTConfig.hpp)
add_custom_target(
  quicktype_bt
  COMMAND
    quicktype ../configs/bt_config.json -l cpp -o configs/BTConfig.hpp
    --no-boost --code-format with-struct --top-level MachineConfig &&
    ../scripts/gen_default_config.sh ../configs/bt_config.json
    configs/BTConfig.hpp >> configs/BTConfig.hpp
  COMMENT "Generating src/BTConfig.hpp from configs/bt_config.json"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/)
# Custom target to avoid double build add_custom_target(quicktype_bt DEPENDS
# ${PROJECT_SOURCE_DIR}/src/BTConfig.hpp)

# Generate Generic config class (GenericConfig.hpp)
add_custom_target(
  quicktype_generic
  COMMAND
    quicktype ../configs/generic_config.json -l cpp -o configs/GenericConfig.hpp
    --no-boost --code-format with-struct --top-level MachineConfig &&
    ../scripts/gen_default_config.sh ../configs/generic_config.json
    configs/GenericConfig.hpp >> configs/GenericConfig.hpp
  COMMENT "Generating src/GenericConfig.hpp from configs/generic_config.json"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/)
# Custom target to avoid double build add_custom_target(quicktype_generic
# DEPENDS ${PROJECT_SOURCE_DIR}/src/GenericConfig.hpp)

# Generate Wi-Fi AP config class (WifiAPConfig.hpp)
add_custom_target(
  quicktype_wifi_ap
  COMMAND
    quicktype ../configs/wifi_ap_config.json -l cpp -o configs/WifiAPConfig.hpp
    --no-boost --code-format with-struct --top-level MachineConfig &&
    ../scripts/gen_default_config.sh ../configs/wifi_ap_config.json
    configs/WifiAPConfig.hpp >> configs/WifiAPConfig.hpp
  COMMENT "Generating src/WifiAPConfig.hpp from configs/wifi_ap_config.json"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/)
# Custom target to avoid double build add_custom_target(quicktype_wifi_ap
# DEPENDS ${PROJECT_SOURCE_DIR}/src/WifiAPConfig.hpp)

# Generate BT Host config class (BTHostConfig.hpp)
add_custom_target(
  quicktype_bthost
  COMMAND
    quicktype ../configs/bthost_config.json -l cpp -o configs/BTHostConfig.hpp
    --no-boost --code-format with-struct --top-level MachineConfig &&
    ../scripts/gen_default_config.sh ../configs/bthost_config.json
    configs/BTHostConfig.hpp >> configs/BTHostConfig.hpp
  COMMENT "Generating src/BTHostConfig.hpp from configs/bthost_config.json"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/)
# Custom target to avoid double build add_custom_target(quicktype_wifi_ap
# DEPENDS ${PROJECT_SOURCE_DIR}/src/WifiAPConfig.hpp)
# Generate Python modules for wdissector (wdissector.py)
add_custom_command(
  TARGET wdissector
  POST_BUILD
  COMMAND
    ctypesgen -DGEN_PY_MODULE ${PROJECT_SOURCE_DIR}/src/wdissector.h --strip-build-path=${PROJECT_SOURCE_DIR}/ 
    -Lbin -lbin/libwdissector.so -o ${PROJECT_SOURCE_DIR}/bindings/python/wdissector.py &>
    /dev/null && ln -sf ../../bin/ ${PROJECT_SOURCE_DIR}/bindings/python/bin
  BYPRODUCTS ${PROJECT_SOURCE_DIR}/bindings/python/wdissector.py)

# Generate GO package for wdissector (wdissector.go)
if(SWIG_EXIST)
  log(INFO "GO and SWIG found: Building GO Package")
  set(INSTALL_GO_PACKAGE TRUE)
  add_custom_command(
    TARGET wdissector
    POST_BUILD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bindings/go/wdissector/
    COMMAND
      cp ${PROJECT_SOURCE_DIR}/src/wdissector.h wdissector.h && swig -go -cgo
      -intgosize 64 -I./libs/wireshark/ `pkg-config --cflags-only-I glib-2.0` wdissector.i 
      && sed -i "'/import \"C\"/i\/\/ #cgo CFLAGS: -Ilibs/wireshark/'" wdissector.go 
      && sed -i "'/import \"C\"/i\/\/ #cgo pkg-config: glib-2.0'" wdissector.go 
      && sed -i "'/import \"C\"/i\/\/ #cgo LDFLAGS: -L./bin -Wl,-rpath=\$$ORIGIN/bin -lwdissector'"
      wdissector.go 
      && ln -sf ../../bin/ ${PROJECT_SOURCE_DIR}/bindings/go/bin 
      && ln -sf ../../libs/ ${PROJECT_SOURCE_DIR}/bindings/go/libs
    BYPRODUCTS ${PROJECT_SOURCE_DIR}/bindings/go/wdissector/wdissector.h
               ${PROJECT_SOURCE_DIR}/bindings/go/wdissector/wdissector.go
               ${PROJECT_SOURCE_DIR}/bindings/go/wdissector/wdissector_wrap.c)
endif()

# ---------------- INSTALL Config --------------------
# CMAKE_INSTALL_PREFIX is /usr/local by default
log(INFO "Build RPath: ${CMAKE_BUILD_RPATH}")

export(
  TARGETS wdissector wireapi
  NAMESPACE wdissector::
  FILE ${PROJECT_BINARY_DIR}/wdissector-targets.cmake)

# Add this at the end of CMakeLists.txt
# Set gatt fuzzer source code files
# set(ZIGBEE_SRC src/zigbee_real_time_fuzzer.cpp libs/shared_memory.c)
# set(5G_SRC src/5g_real_time_fuzzer.cpp libs/shared_memory.c)
set(COAP_SRC src/coap_realtime_fuzzer.cpp libs/shared_memory.c)
# set(MQTT_SRC src/mqtt_realtime_fuzzer.cpp libs/shared_memory.c)

# set(ZIGBEE_SRC src/zigbee_real_time.cpp libs/shared_memory.c)
# Add gatt fuzzer executable
# Zigbee here starts
# add_executable(zigbee_real_time_fuzzer ${ZIGBEE_SRC} libs/profiling.c)
# target_link_libraries(zigbee_real_time_fuzzer PRIVATE ${MINIMAL_FUZZER_LIBS} viface)

# target_compile_options(zigbee_real_time_fuzzer PRIVATE -w -O0)
# target_compile_definitions(zigbee_real_time_fuzzer PRIVATE -DFUZZ_ZNP)

# add_executable(5g_real_time_fuzzer ${5G_SRC} libs/profiling.c)
# target_link_libraries(5g_real_time_fuzzer PRIVATE ${MINIMAL_FUZZER_LIBS} viface)

# target_compile_options(5g_real_time_fuzzer PRIVATE -w -O0)
# target_compile_definitions(5g_real_time_fuzzer PRIVATE -DFUZZ_WIFI_AP)

# target_precompile_headers(zigbee_fuzzer PRIVATE src/PCHBT.hpp)

add_executable(coap_realtime_fuzzer ${COAP_SRC} libs/profiling.c)
target_link_libraries(coap_realtime_fuzzer PRIVATE ${MINIMAL_FUZZER_LIBS} viface)

target_compile_options(coap_realtime_fuzzer PRIVATE -w -O0)
target_compile_definitions(coap_realtime_fuzzer PRIVATE -DFUZZ_WIFI_AP)
# add_executable(mqtt_realtime_fuzzer ${MQTT_SRC} libs/profiling.c)
# target_link_libraries(mqtt_realtime_fuzzer PRIVATE ${MINIMAL_FUZZER_LIBS} viface)

# target_compile_options(mqtt_realtime_fuzzer PRIVATE -w -O0)
# target_compile_definitions(mqtt_realtime_fuzzer PRIVATE -DFUZZ_WIFI_AP)