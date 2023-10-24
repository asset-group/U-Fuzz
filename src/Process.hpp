#pragma once
#ifndef __PROCESS__
#define __PROCESS__

#include "libs/log_misc_utils.hpp"
#include "libs/strtk.hpp"
#include "libs/termcolor.hpp"
#include "libs/tinyprocess/process.hpp"
#include <functional>
#include <iostream>
#include <mutex>
#include <sched.h>
#include <stdlib.h>
#include <string>

using namespace std;

class ProcessRunner {
private:
    shared_ptr<TinyProcessLib::Process> process;
    function<void(const char *bytes, size_t n)> process_callback = nullptr;
    function<void(const char *bytes, size_t n)> error_callback = nullptr;
    function<void()> start_callback = nullptr;
    function<void()> ready_callback = nullptr;
    function<void()> stop_callback = nullptr;
    bool ignore_init_callback = false;
    bool detached = false;
    bool restart_on_exit = false;

    bool start_process()
    {
        if (process_started)
            return false;

        bool ret;
        string process_cmd = process_name;
        vector<string> p_name;
        strtk::parse(process_name, "/", p_name);
        this->process_name_only = p_name.back();
        if (this->change_working_dir) {
            if (p_name.size() > 1) {
                this->process_exec_dir = "";
                for (size_t i = 0; i < p_name.size() - 1; i++) {
                    this->process_exec_dir += p_name[i] + "/";
                }
            }
            else {
                this->process_exec_dir = "./";
            }
        }

        function<void(const char *bytes, size_t n)> stdout_callback = process_callback;
        function<void(const char *bytes, size_t n)> stderr_callback = error_callback;

        if (process_args.size())
            process_cmd += " " + process_args;

        if (stderr == nullptr) {
            stdout_callback = process_callback;
            stderr_callback = process_callback;
        }
        else {
            stdout_callback = process_callback;
            stderr_callback = error_callback;
        }

        if (!this->ignore_init_callback && start_callback)
            start_callback();

        this->process = make_shared<TinyProcessLib::Process>(process_cmd, "", stdout_callback, stderr_callback, process_exec_dir, this->detached);
        process_started = (process != nullptr);

        if (this->restart_on_exit) {

            thread *thread_wait_process = new thread([&]() {
                enable_idle_scheduler();

                while (this->restart_on_exit) {
                    int res = WaitProcess();
                    if (!this->stopping) {
                        // this->stop(true, true);
                        this_thread::sleep_for(chrono::seconds(2));
                        this->process_started = false;
                        this->init();
                    }
                }
            });
            thread_wait_process->detach();
        }

        return process_started;
    }

public:
    string process_name;
    string process_args;
    string process_name_only;
    bool process_started;
    bool stopping = false;
    bool change_working_dir = false;
    string process_exec_dir = "";

    ~ProcessRunner()
    {
        stop(true);
    }

    void SetStartCallback(function<void()> fcn)
    {
        start_callback = fcn;
    }

    void SetReadyCallback(function<void()> fcn)
    {
        ready_callback = fcn;
    }

    void SetStopCallback(function<void()> fcn)
    {
        stop_callback = fcn;
    }

    bool WaitProcess()
    {
        if (!process_started || !this->process)
            return false;
        return this->process->get_exit_status();
    }

    int GetPID()
    {
        if (!process_started || !this->process)
            return -1;

        return this->process->get_id();
    }

    void stop(bool wait = false, bool force = true)
    {
        if (stopping)
            return;

        stopping = true;
        if (stop_callback)
            stop_callback();

        if (this->detached) {
            LOG2("detached:", detached);
            stopping = false;
            return;
        }

        if (!wait) {
            if (this->process && process_started) {
                thread t = thread([&] {
                    // string cmd = "pkill " + process_name_only;
                    // system(cmd.c_str());
                    // LOG1("KILL P");
                    this->process->kill(force);
                    // this->process->get_exit_status();
                    process_started = false;
                    stopping = false;
                });
                t.detach();
            }
        }
        else if (this->process) {
            // string cmd = "pkill " + process_name_only;
            // system(cmd.c_str());
            // LOG1("KILL P");
            this->process->kill(force);
            // this->process->get_exit_status();
            process_started = false;
            stopping = false;
        }
    }

    void restart(bool force = false, bool ignore_init_callback = false)
    {
        if ((process_started && !stopping) || force) {
            this->ignore_init_callback = ignore_init_callback;
            stop(true, force);
            init();
            this->ignore_init_callback = false;
        }
    }

    void setup(string process_name, string args, function<void(const char *bytes, size_t n)> process_callback,
               bool change_working_dir = false, bool restart_on_exit = false)
    {
        this->change_working_dir = change_working_dir;
        this->process_args = args;
        this->process_name = process_name;
        this->process_callback = process_callback;
        this->error_callback = nullptr;
        this->restart_on_exit = restart_on_exit;
    }

    bool init()
    {
        return init(process_name, process_args, process_callback, error_callback, change_working_dir);
    }

    bool init(string process_name, void (*process_callback)(const char *bytes, size_t n), bool change_working_dir = false)
    {
        this->change_working_dir = change_working_dir;
        this->process_args = "";
        this->process_name = process_name;
        this->process_callback = process_callback;
        this->error_callback = nullptr;

        return start_process();
    }

    bool init(string process_name, function<void(const char *bytes, size_t n)> process_callback, function<void(const char *bytes, size_t n)> error_callback,
              bool change_working_dir = false)
    {
        this->change_working_dir = change_working_dir;
        this->process_args = "";
        this->process_name = process_name;
        this->process_callback = process_callback;
        this->error_callback = error_callback;

        return start_process();
    }

    bool init(string process_name, string args, function<void(const char *bytes, size_t n)> process_callback,
              bool change_working_dir = false, bool restart_on_exit = false)
    {
        this->change_working_dir = change_working_dir;
        this->process_args = args;
        this->process_name = process_name;
        this->process_callback = process_callback;
        this->error_callback = process_callback;
        this->restart_on_exit = restart_on_exit;

        return start_process();
    }

    bool init(string process_name, string args, function<void(const char *bytes, size_t n)> process_callback, function<void(const char *bytes, size_t n)> error_callback,
              bool change_working_dir = false)
    {
        this->process_args = args;
        this->process_name = process_name;
        this->process_callback = process_callback;
        this->error_callback = error_callback;
        this->change_working_dir = change_working_dir;

        return start_process();
    }

    void set_args(string args)
    {
        this->process_args = args;
    }

    void setDetached(bool en)
    {
        this->detached = en;
    }

    bool isRunning()
    {
        if (this->process) {
            int ec;
            bool r = this->process->try_get_exit_status(ec);
            this->process_started = (r == 0);
            return this->process_started;
        }
        else {
            return false;
        }
    }
};

#endif