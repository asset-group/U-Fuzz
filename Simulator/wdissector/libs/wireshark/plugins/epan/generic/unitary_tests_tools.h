/*
 * Copyright 2005-2019 Olivier Aveline <wsgd@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef UNITARY_TEST_TOOLS_H
#define UNITARY_TEST_TOOLS_H

//*****************************************************************************
// Includes
//*****************************************************************************

#include <cassert>
#include <iostream>
#include <sstream>
using namespace std;

#include "byte_interpret_common_utils.h"
#include "T_static_executor.h"
#include "T_static_executor_manager.h"

#define wait_for_any_operator_input_if_necessary()


//*****************************************************************************
// Counter of tests done.
// Counter of errors detected.
// Counter of errors detected already known (minor errors).
//*****************************************************************************

extern int   G_TEST_nb_tests;
extern int   G_TEST_nb_errors;
extern int   G_TEST_nb_errors_already_known;


//*****************************************************************************
// Declare a function containing test
// The function will be called automatically
//*****************************************************************************
#define M_TEST_FCT(fct_name)                                                  \
extern void fct_name();                                                       \
M_static_executor(fct_name);                                                  \
void fct_name()


// The function will NOT be called automatically
#define M_TEST_FCT_IGNORE(fct_name)                                           \
void fct_name()


//*****************************************************************************
// Test macro.
// - increment "Counter of tests done"
// - verify assertion, if wrong :
//   - output ERROR trace
//   - increment "Counter of errors detected"
//*****************************************************************************

#define M_TEST(ASSERTION)                                                     \
    ++G_TEST_nb_tests;                                                        \
    if ((ASSERTION) != true)                                                  \
    {                                                                         \
        ++G_TEST_nb_errors;                                                   \
        cout << "ERROR in test " << #ASSERTION << endl                        \
             << " at " << __FILE__ << "[" << __LINE__ << "]"                  \
             << " test# " << G_TEST_nb_tests                                  \
             << endl;                                                         \
    }

//*****************************************************************************
// Test macro which compare 2 values.
//*****************************************************************************

#define M_TEST_EQ(VAL1,VAL2)    M_TEST_fail_compare(VAL1, !=, VAL2)
#define M_TEST_NE(VAL1,VAL2)    M_TEST_fail_compare(VAL1, ==, VAL2)
#define M_TEST_GE(VAL1,VAL2)    M_TEST_fail_compare(VAL1, < , VAL2)
#define M_TEST_GT(VAL1,VAL2)    M_TEST_fail_compare(VAL1, <=, VAL2)
#define M_TEST_LE(VAL1,VAL2)    M_TEST_fail_compare(VAL1, > , VAL2)
#define M_TEST_LT(VAL1,VAL2)    M_TEST_fail_compare(VAL1, >=, VAL2)

#define M_TEST_TRIM_EQ(VAL1,VAL2)                                             \
    ++G_TEST_nb_tests;                                                        \
    if (VAL1 != VAL2)                                                         \
        if (get_trim(VAL1) != get_trim(VAL2))                                 \
            M_TEST_failure(VAL1, !=, VAL2)

//*****************************************************************************
// Test macro for pointer
//*****************************************************************************
std::ostream& operator << (std::ostream & os, std::nullptr_t);

#define M_TEST_NULL(PTR)        M_TEST_EQ(PTR, nullptr)
#define M_TEST_NOT_NULL(PTR)    M_TEST_NE(PTR, nullptr)

//*****************************************************************************
// Test macro.
// - increment "Counter of tests done"
// - assert assertion
//*****************************************************************************

#define M_TEST_assert(ASSERTION)                                              \
    ++G_TEST_nb_tests;                                                        \
    assert (ASSERTION)

//*****************************************************************************
// Test macro.
// - verify that the specified exception is throwed
//*****************************************************************************

#define M_TEST_CATCH_EXCEPTION(instruction_that_calls_function,exception)     \
{                                                                             \
  bool  is_exception_catched = false;                                         \
                                                                              \
  try                                                                         \
  {                                                                           \
    instruction_that_calls_function;                                          \
  }                                                                           \
  catch (exception&)                                                          \
  {                                                                           \
    is_exception_catched = true;                                              \
  }                                                                           \
                                                                              \
  M_TEST_EQ (is_exception_catched, true);                                     \
}


//*****************************************************************************
// Synthesis macro
// - output end of main
// - output synthesis information
// - return the "Counter of errors detected"
//*****************************************************************************

#define M_TEST_RETURN_OF_MAIN()                                               \
    cout << "End of main" << endl;                                            \
                                                                              \
    string  TEST_new_label = "";                                              \
    if (G_TEST_nb_errors_already_known != 0)                                  \
    {                                                                         \
        TEST_new_label = " new";                                              \
        cout << "         " << G_TEST_nb_errors_already_known                 \
             << " already known errors." << endl;                             \
    }                                                                         \
    if (G_TEST_nb_errors == 0)                                                \
        cout << "    OK : no" << TEST_new_label << " error";                  \
    else                                                                      \
        cout << "NOT OK : " << G_TEST_nb_errors << TEST_new_label << " errors";  \
    cout << " found during " << G_TEST_nb_tests << " tests." << endl;         \
                                                                              \
    wait_for_any_operator_input_if_necessary ();                              \
                                                                              \
    return  G_TEST_nb_errors


//*****************************************************************************
//*****************************************************************************
//! IMPLEMENTATION --- Do not read --- IMPLEMENTATION --- Do not read ---
//! IMPLEMENTATION --- Do not read --- IMPLEMENTATION --- Do not read ---
//! IMPLEMENTATION --- Do not read --- IMPLEMENTATION --- Do not read ---
//*****************************************************************************
//*****************************************************************************

//! The specified comparison must fail for the test be valid.
#define M_TEST_fail_compare(VAL1,FAIL_COMPARE,VAL2)                           \
    ++G_TEST_nb_tests;                                                        \
    if (VAL1 FAIL_COMPARE VAL2)                                               \
    M_TEST_failure(VAL1,FAIL_COMPARE,VAL2)

//! For float/double comparison.
//! Could NOT work for little values (like zero).
#define M_TEST_EQ_APPROX(VAL1,VAL2)                                           \
    ++G_TEST_nb_tests;                                                        \
    if (fabs(VAL1 - VAL2) > fabs(VAL1 / 100000.0))                            \
    M_TEST_failure(VAL1,"NOT approx =",VAL2)



//*****************************************************************************
// error_already_known flag 
//*****************************************************************************

extern bool         G_TEST_is_an_error_already_known;
extern std::string  G_TEST_error_already_known_explain;
class C_TEST_error_already_known
{
public:
    C_TEST_error_already_known(long  bug_id, const std::string  & comment)
        :A_save_G_TEST_nb_errors_already_known(G_TEST_nb_errors_already_known)
    {
        G_TEST_is_an_error_already_known = true;
        G_TEST_error_already_known_explain = "";

        {
            std::ostringstream  oss;
            if (bug_id > 0)
            {
                oss << "bug=" << bug_id << "  ";
            }
            oss << comment;
            G_TEST_error_already_known_explain += oss.str();
        }
    }

    ~C_TEST_error_already_known()
    {
        G_TEST_is_an_error_already_known = false;
        G_TEST_error_already_known_explain = "";
        if (G_TEST_nb_errors_already_known <= A_save_G_TEST_nb_errors_already_known)
        {
            ++G_TEST_nb_errors;
            cout << "NO MORE ERROR in test" << endl;
        }
    }

private:
    int          A_save_G_TEST_nb_errors_already_known;
};

#define M_TEST_ERROR_ALREADY_KNOWN__OPEN(BUGID,COMMENT)                       \
{ C_TEST_error_already_known  TEST_error_already_known(BUGID,COMMENT);

#define M_TEST_EXCEPTION_ALREADY_KNOWN(instruction_that_calls_function)       \
{                                                                             \
  bool  is_exception_catched = false;                                         \
                                                                              \
  try                                                                         \
  {                                                                           \
    instruction_that_calls_function;                                          \
  }                                                                           \
  catch (...)                                                                 \
  {                                                                           \
    is_exception_catched = true;                                              \
    ++G_TEST_nb_errors_already_known;                                         \
    cout << "EXCEPTION (already known: "                                      \
         << G_TEST_error_already_known_explain << ") "                        \
         << " at " << __FILE__ << "[" << __LINE__ << "]"                      \
         << " test# " << G_TEST_nb_tests                                      \
         << endl;                                                             \
  }                                                                           \
                                                                              \
  M_TEST_EQ (is_exception_catched, true);                                     \
}

//*****************************************************************************
// Error management
//*****************************************************************************

//! The specified fail comparison is valid, so the test is not valid.
#define M_TEST_failure(VAL1,FAIL_COMPARE,VAL2)                                \
    {                                                                         \
        string  TEST_already_known_label = "";                                \
        if (G_TEST_is_an_error_already_known == true)                         \
        {                                                                     \
            ++G_TEST_nb_errors_already_known;                                 \
            TEST_already_known_label = "(already known: ";                    \
            TEST_already_known_label += G_TEST_error_already_known_explain;   \
            TEST_already_known_label += ") ";                                 \
        }                                                                     \
        else                                                                  \
            ++G_TEST_nb_errors;                                               \
        cout << "ERROR " << TEST_already_known_label << "in test "            \
             << #VAL1 " (" << VAL1 << ") "                                    \
             << #FAIL_COMPARE " "                                             \
             << #VAL2 " (" << VAL2 << ") " << endl                            \
             << " at " << __FILE__ << "[" << __LINE__ << "]"                  \
             << " test# " << G_TEST_nb_tests                                  \
             << endl;                                                         \
    }

#if 0
 following test could NOT be added,
  because it could have many tests and only 1 (or few) fails.

    else if (G_TEST_is_an_error_already_known == true)                        \
    {                                                                         \
        ++G_TEST_nb_errors;                                                   \
        cout << "NO MORE ERROR in test "                                      \
             << #VAL1 " (" << VAL1 << ") "                                    \
             << #FAIL_COMPARE " "                                             \
             << #VAL2 " (" << VAL2 << ") " << endl                            \
             << " at " << __FILE__ << "[" << __LINE__ << "]"                  \
             << " test# " << G_TEST_nb_tests                                  \
             << endl;                                                         \
    }
#endif


#endif
