#pragma once
#ifndef __PCH__
#define __PCH__

// Insert libraries that are used in the project here
// If the header of such libries are changed, the PCH must be regenerated again

#ifdef __cplusplus
#define STB_IMAGE_IMPLEMENTATION
#define FUZZ_BT 1

#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include <pty.h>
#include <termios.h>

#include <config.h>     // glib 2.0
#include <epan/proto.h> // Wireshark include
#include <glib.h>       // glib 2.0

#include "libs/json.hpp"
#include "libs/log_misc_utils.hpp"
#include "libs/mv_average.hpp"
#include "libs/oai_tracing/T_IDs.h"
#include "libs/refl.hpp"
#include "libs/termcolor.hpp"
#include "libs/tinyprocess/process.hpp"
#include "libs/zmq.hpp"
#include "serial/serial.h"
// imgui includes
#include "libs/gvpp/gvpp.hpp"
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_glfw.h"
#include "libs/imgui/imgui_impl_opengl3.h"
#include "libs/imgui/imgui_memory_editor.hpp"
#include "libs/imgui/imguial_term.h"
#include "libs/react-cpp/reactcpp.h"
#include "libs/strtk.hpp"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <graphviz/gvc.h>
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>
#include <include/wrapper/cef_resource_manager.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
// #include "GUI_BLUETOOTH.hpp"
#include "MiscUtils.hpp"
#include "Process.hpp"
// #include "Machine.hpp"

extern "C" {
#include "libs/logs.h"
#include "libs/profiling.h"
#include "libs/shared_memory.h"
#include "wdissector.h"
}

#else
#include "libs/logs.h"
#include "libs/oai_tracing/T_IDs.h"
#include "libs/profiling.h"
#include "libs/shared_memory.h"
#include "wdissector.h"
#include <config.h>
#include <glib.h>
#include <inttypes.h>
#include <unistd.h>
#endif

#endif
