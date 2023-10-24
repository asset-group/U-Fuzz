#pragma once

#include <vector>
#include <string>
#include "T_static_executor.h"


class T_static_executor_manager
{
public:
    static T_static_executor_manager& getInstance();

    void add(T_static_executor& static_executor);

    // Return the number of executions done
    // Remaining arguments permits to choose executors to execute
    // - file_name.cpp   : execute executors defined into file which ends by ...
    // - function_name   : execute executor called ...
    // - function_name*  : execute executors beginning by ...
    int execute(const int           argc,
                const char * const  argv[],
                const int           arg_idx);

public:  // for UT
    struct T_execution_filters
    {
        std::vector<std::string>  filter_file_name_ends;
        std::vector<std::string>  filter_function_names;
        std::vector<std::string>  filter_function_name_starts;

        bool  must_execute(const T_static_executor& executor) const;
    };

    static
    T_execution_filters  build_execution_filters(const int           argc,
                                                 const char * const  argv[],
                                                 const int           arg_idx);

private:
    std::vector<T_static_executor> m_static_executors;
};
