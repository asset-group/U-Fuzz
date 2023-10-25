#pragma once

#include <string>


// FCT :
// - have NO parameters
// - returns nothing
#define M_static_executor(FCT)  \
static T_static_executor M_static_executor_##FCT(FCT, std::string(__FILE__), #FCT);


class T_static_executor
{
public:
    T_static_executor(void(*test_fct)(), const std::string& file_name, const std::string& fct_name)
        : m_test_fct(test_fct)
        , m_file_name(file_name)
        , m_fct_name(fct_name)
        , m_label(file_name + " " + fct_name)
    {
        do_register();
    }

    void execute();

    const std::string &  get_fct_name() const { return m_fct_name; }
    const std::string &  get_file_name() const { return m_file_name; }

private:
    void do_register();

    void(*m_test_fct)() = nullptr;
    std::string m_file_name;
    std::string m_fct_name;
    std::string m_label;
};
