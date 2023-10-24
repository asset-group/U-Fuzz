#pragma once

#ifndef __GUIIMG__
#define __GUIIMG__
// imgui includes
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_internal.h"
#include "libs/imgui/imguial_term.h"
#include "libs/log_misc_utils.hpp"
#include "libs/mv_average.hpp"
#include <fstream>
#include <mutex>
#include <queue>
#include <semaphore.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "libs/imgui/imgui_impl_glfw.h"
#include "libs/imgui/imgui_impl_opengl3.h"
#include "libs/imgui/imgui_memory_editor.hpp"
#include "libs/imgui/misc/fonts/DroidSans.hpp"
#include "libs/imgui/misc/fonts/DroidSansMono.hpp"
#include "libs/imgui/misc/fonts/IconsFontAwesome5.hpp"
#include "libs/imgui/misc/freetype/imgui_freetype.h"

extern "C" {
#include "libs/profiling.h"
}

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

#define GL1(...) gui_log1.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, __VA_ARGS__)
#define GL1C(...) gui_log1.add_msg_color(ImGuiAl::Crt::CGA::BrightCyan, __VA_ARGS__)
#define GL1M(...) gui_log1.add_msg_color(ImGuiAl::Crt::CGA::BrightMagenta, __VA_ARGS__)
#define GL1G(...) gui_log1.add_msg_info(__VA_ARGS__)
#define GL1Y(...) gui_log1.add_msg_warning(__VA_ARGS__)
#define GL1R(...) gui_log1.add_msg_error(__VA_ARGS__)

#define GL2(...) gui_log2.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, __VA_ARGS__)
#define GL2C(...) gui_log2.add_msg_color(ImGuiAl::Crt::CGA::BrightCyan, __VA_ARGS__)
#define GL2M(...) gui_log2.add_msg_color(ImGuiAl::Crt::CGA::BrightMagenta, __VA_ARGS__)
#define GL2G(...) gui_log2.add_msg_info(__VA_ARGS__)
#define GL2Y(...) gui_log2.add_msg_warning(__VA_ARGS__)
#define GL2R(...) gui_log2.add_msg_error(__VA_ARGS__)

#define GL3(...) gui_log3.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, __VA_ARGS__)
#define GL3C(...) gui_log3.add_msg_color(ImGuiAl::Crt::CGA::BrightCyan, __VA_ARGS__)
#define GL3M(...) gui_log3.add_msg_color(ImGuiAl::Crt::CGA::BrightMagenta, __VA_ARGS__)
#define GL3G(...) gui_log3.add_msg_info(__VA_ARGS__)
#define GL3Y(...) gui_log3.add_msg_warning(__VA_ARGS__)
#define GL3R(...) gui_log3.add_msg_error(__VA_ARGS__)

#define GL4(...) gui_log4.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, __VA_ARGS__)
#define GL4C(...) gui_log4.add_msg_color(ImGuiAl::Crt::CGA::BrightCyan, __VA_ARGS__)
#define GL4M(...) gui_log4.add_msg_color(ImGuiAl::Crt::CGA::BrightMagenta, __VA_ARGS__)
#define GL4G(...) gui_log4.add_msg_info(__VA_ARGS__)
#define GL4Y(...) gui_log4.add_msg_warning(__VA_ARGS__)
#define GL4R(...) gui_log4.add_msg_error(__VA_ARGS__)

using namespace std::chrono_literals;
using namespace std;

#define GUI_APP_NAME "Wi-Fi AP Fuzzer"
#define GUI_LOG1_NAME ICON_FA_EXCLAMATION_CIRCLE " Events"
#define GUI_LOG2_NAME ICON_FA_EXCHANGE_ALT " TX / RX"
#define GUI_LOG3_NAME ICON_FA_CHART_BAR " Summary"
#define GUI_LOG4_NAME ICON_FA_BLUETOOTH_B " WiFiProcess"
#define GUI_LOG5_NAME ICON_FA_HEARTBEAT " Monitor"
#define GUI_CONFIG_PATH "/tmp/wifi_fuzz_ap_gui.bin"
#define GUI_LOG_LIMIT 8000   // Max lines
#define GUI_SUMMARY_LIMIT 10 // Max lines for summary

#define GUI_UPDATE_INTERVAL 0.033s // About 30fps
#define GUI_GLSL_VERSION "#version 130"

ImGuiAl::Log gui_log1(GUI_LOG_LIMIT, GUI_LOG1_NAME, true);
ImGuiAl::Log gui_log2(GUI_LOG_LIMIT, GUI_LOG2_NAME, false, false, true);
ImGuiAl::Log gui_log3(GUI_SUMMARY_LIMIT, GUI_LOG3_NAME, false, true);
ImGuiAl::Log gui_log4(GUI_LOG_LIMIT, GUI_LOG4_NAME);
ImGuiAl::Log gui_log5(GUI_LOG_LIMIT, GUI_LOG5_NAME);

int GUI_Init(int argc, char **argv, bool enable = true)
{

    gui_log1.forceHorizontalScrollbar(true);
    gui_log2.forceHorizontalScrollbar(true);
    gui_log3.setHideOptions(true);
    gui_log3.forceHorizontalScrollbar(true);
    gui_log4.forceHorizontalScrollbar(true);
    gui_log5.forceHorizontalScrollbar(true);
    return 0;
}
#endif