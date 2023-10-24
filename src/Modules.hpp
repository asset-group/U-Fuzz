#pragma once

#ifndef __MODULES_WD__
#define __MODULES_WD__

#include "Machine.hpp"
#include "ModulesInclude.hpp"
#include "libs/dynalo/include/dynalo/dynalo.hpp"
#include "libs/tinydir.h"
#include "wdissector.h"
#include <algorithm>
#include <inttypes.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace misc_utils;

#define timespeccmp(a, b, CMP)            \
    (((a)->tv_sec == (b)->tv_sec)         \
         ? ((a)->tv_nsec CMP(b)->tv_nsec) \
         : ((a)->tv_sec CMP(b)->tv_sec))

typedef struct
{
    bool enable;
    bool require_setup;
    string name;
    const char *(*module_name)() = NULL;
    int (*setup)(void *p) = NULL;
    int (*tx_pre_dissection)(uint8_t *pkt_buf, int pkt_length, void *p) = NULL;
    int (*tx_post_dissection)(uint8_t *pkt_buf, int pkt_length, void *p) = NULL;
    int (*rx_pre_dissection)(uint8_t *pkt_buf, int pkt_length, void *p) = NULL;
    int (*rx_post_dissection)(uint8_t *pkt_buf, int pkt_length, void *p) = NULL;
    dynalo::library *lib;
    bool configure(bool enable, void *p = nullptr)
    {
        bool ret = false;

        if (enable && this->require_setup) {
            this->require_setup = false;

            if (!this->setup(p)) {
                GL1G("[Modules] ", name, " configured and ready!");
                this->enable = true;
                ret = true;
            }
            else {
                GL1R("[Modules] ", name, ".so error (setup returns != 0)");
            }
        }
        else {
            this->require_setup = true;
            this->enable = false;
            ret = true;
        }

        return ret;
    }

} wdmodules_func_t;

class WDModules {
private:
    void *user_ptr;

    bool any_enabled;

    string gen_module_glue(const string source_path)
    {
        stringstream glue;
        string module_name = string_split(source_path.c_str(), "/").back();
        ifstream ifs(source_path + ".cpp");
        if (ifs.good()) {
            glue << ifs.rdbuf();
            // Add glue code
            glue << "\nextern \"C\" const char * " + module_name + "_module_name(){return module_name();}";
            glue << "\nextern \"C\" int " + module_name + "_setup(void *p){return setup(p);}";
            glue << "\nextern \"C\" int " + module_name + "_tx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p){return tx_pre_dissection(pkt_buf,pkt_length,p);}";
            glue << "\nextern \"C\" int " + module_name + "_tx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p){return tx_post_dissection(pkt_buf,pkt_length,p);}";
            glue << "\nextern \"C\" int " + module_name + "_rx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p){return rx_pre_dissection(pkt_buf,pkt_length,p);}";
            glue << "\nextern \"C\" int " + module_name + "_rx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p){return rx_post_dissection(pkt_buf,pkt_length,p);}";
            ifs.close();
        }
        else
            return "";
        return glue.str();
    }

    bool was_modified(const string source_path)
    {
        struct stat attrib1, attrib2;

        if (stat((source_path + ".cpp").c_str(), &attrib1))
            return false;

        if (stat((source_path + ".so").c_str(), &attrib2))
            return true;

        return timespeccmp(&attrib1.st_mtim, &attrib2.st_mtim, >);
    }

public:
    int modules_count;
    int modules_compiled;
    int modules_loaded;
    bool has_gcc;

    unordered_map<string, wdmodules_func_t> modules_map;
    vector<wdmodules_func_t *> modules_ptr;

    const char *TAG = "[Modules] ";

    bool init(const char *modules_path, void *custom_ptr = nullptr)
    {
        any_enabled = false;
        modules_count = 0;
        modules_compiled = 0;
        modules_loaded = 0;
        has_gcc = false;
        bool no_err = true;
        this->user_ptr = custom_ptr;

        GL1Y("[Modules] Loading C++ Modules...");

        if (modules_path == NULL)
            return false;

        if (!system("sudo which g++ > /dev/null 2>&1"))
            has_gcc = true;
        else
            LOGY("GCC not found, modules won't be compiled from source.");

        tinydir_dir dir;

        if (tinydir_open(&dir, modules_path) == -1)
            return false;

        tinydir_file file;

        // First pass check cpps and compile modules (.so)
        while (dir.has_next) {
            if (!tinydir_readfile(&dir, &file)) {
                if (file.is_dir) {
                    tinydir_next(&dir);
                    continue;
                }

                if (!strcmp(file.extension, "cpp")) {
                    string module_name = string_split(file.name, ".")[0];
                    string source_path = modules_path + "/"s + module_name;
                    // Compile module
                    if (has_gcc && was_modified(source_path)) {
                        system(("g++ -std=c++17 -fPIC -w -O3 src/ModulesStub.cpp -c -o "s + modules_path + "/ModulesStub.o").c_str());
                        GL1G("Compiling ", module_name, ".cpp");
                        // Compile obj
                        string g_cmd_obj = "g++ -x c++ -std=c++17 -fPIC -w -O3 -c "s +
                                           "-o " + source_path + ".o " +
                                           "-I src/ -I libs/ -I libs/wireshark -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include " +
#ifdef FUZZ_BT
                                           "-DFUZZ_BT " +
#elif FUZZ_BTHOST
                                           "-DFUZZ_BTHOST " +
#elif FUZZ_WIFI_AP
                                           "-DFUZZ_WIFI_AP " +
#elif FUZZ_LTE
                                           "-DFUZZ_LTE " +
#endif
                                           "-";

                        LOG1(g_cmd_obj);

                        FILE *p_fd = popen(g_cmd_obj.c_str(), "w");
                        // Add api glue to code (optional)
                        string gen_code = gen_module_glue(source_path);
                        fwrite(&gen_code[0], 1, gen_code.size(), p_fd);

                        if (!pclose(p_fd))
                            modules_compiled++;
                        else
                            no_err = false;

                        // Compile shared library
                        string g_cmd_shared = "g++ -std=c++17 -fPIC -w -O3 -shared -o "s + source_path + ".so " +
                                              source_path + ".o " + modules_path + "/ModulesStub.o " +
                                              "-Lbin/ -Wl,-rpath,'$ORIGIN/bin/' -Wl,-rpath,'$ORIGIN/../../../bin/' "
                                              "-lwdissector -lwireshark";

                        LOG1(g_cmd_shared);

                        if (system(g_cmd_shared.c_str()))
                            no_err = false;
                    }
                }
            }

            tinydir_next(&dir);
        }
        tinydir_close(&dir);

        if (tinydir_open(&dir, modules_path) == -1)
            return false;

        // Second pass, load modules
        while (dir.has_next) {
            if (!tinydir_readfile(&dir, &file)) {
                if (file.is_dir) {
                    tinydir_next(&dir);
                    continue;
                }

                if (!strcmp(file.extension, "so")) {
                    string module_name = string_split(file.name, ".")[0];
                    string source_path = modules_path + "/"s + module_name;

                    try {
                        wdmodules_func_t m;
                        m.lib = new dynalo::library(source_path + ".so");
                        m.enable = false;
                        m.module_name = m.lib->get_function<const char *()>(module_name + "_module_name");
                        m.setup = m.lib->get_function<int(void *)>(module_name + "_setup");
                        m.tx_pre_dissection = m.lib->get_function<int(uint8_t *, int, void *)>(module_name + "_tx_pre_dissection");
                        m.tx_post_dissection = m.lib->get_function<int(uint8_t *, int, void *)>(module_name + "_tx_post_dissection");
                        m.rx_pre_dissection = m.lib->get_function<int(uint8_t *, int, void *)>(module_name + "_rx_pre_dissection");
                        m.rx_post_dissection = m.lib->get_function<int(uint8_t *, int, void *)>(module_name + "_rx_post_dissection");
                        m.name = module_name;
                        m.require_setup = true;

                        // Register module
                        modules_map[m.name] = m;
                        modules_ptr.push_back(&modules_map[m.name]);
                        GL1Y("[Modules] --> " + module_name + ".so loaded");
                        modules_loaded++;
                    }
                    catch (const std::exception &e) {
                        no_err = false;
                        std::cerr << e.what() << '\n';
                    }

                    modules_count++;
                }
            }

            tinydir_next(&dir);
        }

        return no_err;
    }

    bool enable_module(const string module_name, bool enable, void *ptr = nullptr)
    {
        auto module_iter = modules_map.find(module_name);

        if (module_iter != modules_map.end()) {
            if (ptr == nullptr)
                ptr = user_ptr;
            module_iter->second.configure(enable, ptr);
            if (enable)
                any_enabled = true;
            return true;
        }

        return false;
    }

    void printAvailableModules()
    {
        LOGY("Available Exploits:");
        for (auto &m : modules_ptr) {
            LOG3Y("--> '", m->name, "'");
        }
    }

    void DisableAllModules()
    {
        for (auto &m : modules_ptr) {
            m->enable = false;
        }
        any_enabled = false;
    }

    inline int run_tx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p = NULL)
    {
        if (!any_enabled)
            return 0;

        int ret = 0;
        for (auto &m : modules_ptr) {
            if (m->enable) {
                ret |= m->tx_pre_dissection(pkt_buf, pkt_length, p);
            }
        }

        return ret;
    }

    inline int run_tx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p = NULL)
    {
        if (!any_enabled)
            return 0;

        int ret = 0;
        for (auto &m : modules_ptr) {
            if (m->enable) {
                ret |= m->tx_post_dissection(pkt_buf, pkt_length, p);
            }
        }

        return ret;
    }

    inline int run_rx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p = NULL)
    {
        if (!any_enabled)
            return 0;

        int ret = 0;
        for (auto &m : modules_ptr) {
            if (m->enable) {
                ret |= m->rx_pre_dissection(pkt_buf, pkt_length, p);
            }
        }

        return ret;
    }

    inline int run_rx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p = NULL)
    {
        if (!any_enabled)
            return 0;

        int ret = 0;
        for (auto &m : modules_ptr) {
            if (m->enable) {
                ret |= m->rx_post_dissection(pkt_buf, pkt_length, p);
            }
        }

        return ret;
    }
};

#endif