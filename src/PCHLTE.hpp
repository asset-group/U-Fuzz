#ifndef __PCH__
#define __PCH__

// Insert libraries that are used in the project here
// If the header of such libries are changed, the PCH must be regenerated again

#ifdef __cplusplus
#define STB_IMAGE_IMPLEMENTATION

#include <algorithm>
#include <chrono>
#include <config.h> // glib 2.0
#include <csignal>
#include <epan/proto.h> // Wireshark include
#include <fstream>
#include <glib.h> // glib 2.0
#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "json.hpp"
#include "libs/log_misc_utils.hpp"
#include "libs/mv_average.hpp"
#include "libs/oai_tracing/T_IDs.h"
#include "libs/refl.hpp"
#include "libs/termcolor.hpp"
#include "libs/zmq.hpp"
// imgui includes
#include "libs/gvpp/gvpp.hpp"
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_glfw.h"
#include "libs/imgui/imgui_impl_opengl3.h"
#include "libs/imgui/imgui_memory_editor.hpp"
#include "libs/imgui/imguial_term.h"
#include "libs/strtk.hpp"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <graphviz/gvc.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#ifdef FUZZ_BT
// #include "GUI_BLUETOOTH.hpp"
// #include "ESP32BTDriver.hpp"
#endif

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
