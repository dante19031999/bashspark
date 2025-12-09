/**
 * @file test_shell.cpp
 * @brief Implements a series of tests to check the overall compliance of the shell.
 * Exhaustive testing would be way too long.
 *
 * @date Created on 2/11/25
 * @author Dante Doménech Martínez
 *
 * @copyright GNU General Public License v3.0
 *
 * This file is part of BashSpark.
 * Copyright (C) 2025 Dante Doménech Martínez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "BashSpark/test/test_shell.h"

#include <cassert>
#include <sstream>
#include <string_view>

#include "BashSpark/shell/shell_node_visitor_json.h"
#include "BashSpark/tools/nullstream.h"

using namespace std::string_view_literals;

namespace bs::debug {
    namespace {
        /**
         * @brief Custom assertion function
         * @param bCondition Condition to assert
         * @param sMessage Message displayed on fail
         */
        void custom_assert(const bool bCondition, const std::string &sMessage) {
            if (!bCondition) {
                std::cerr << "Assertion failed: " << sMessage << std::endl;
                std::abort();
            }
        }
    }

    test_shell::test_shell()
        : m_pShell(shell::make_default_shell()) {
    }

    void test_shell::test() const {
        this->test_basic();
        this->test_echo_basic();
        this->test_echo_envvar();
        this->test_seq();
        this->test_setenv();
        this->test_setvar();
        this->test_oper();
        this->test_structure();
        this->test_math();
        this->test_test();
        this->test_script();
        std::cout << "Tests finished" << std::endl;
    }

    void test_shell::test_basic() const {
        const std::vector<std::pair<std::string, std::string> > vTests = {
            {"", ""},
            {"()", ""},
            {"{}", ""},
            {";;;", ""},
        };

        inullstream oStdIn;
        onullstream oStdErr;

        for (const auto &[sCommand, sOutput]: vTests) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, oStdErr);
            shell::run(sCommand, oSession);
            const std::string sName = "Check command " + sCommand + " - <" + oStdOut.str() + ">";
            custom_assert(sOutput == oStdOut.view(), sName);
        }
    }


    void test_shell::test_echo_basic() const {
        const std::vector<std::pair<std::string, std::string> > vTests = {
            {"echo", "\n"},
            {"echo -n", ""},
            {"echo 'Hello World!'", "Hello World!\n"},
            {"echo \"Hello World!\"", "Hello World!\n"},
            {"echo -n 'Hello World!'", "Hello World!"},
            {"echo -n \"Hello World!\"", "Hello World!"},
            {"echo -n \\\n 'Hello World!'", "Hello World!"},
            {"echo -n '$var'", "$var"},
            {"echo -n a; echo -n b;", "ab"},
            {"echo -n a && echo -n b;", "ab"},
            {"echo -n a || echo -n b;", "a"},
            {R"(echo -n "\n\t\\\"\'")", "\n\t\\\"\'"},
            {R"(echo -n "\x44\u2205\U00002205\uD83D\uDE00")", "\x44\u2205\U00002205\U0001F600"},
            {"( echo -n )", ""},
            {"( echo -n  'Hello World!' )", "Hello World!"},
            {"{ echo -n }", ""},
            {"{ echo -n  'Hello World!' }", "Hello World!"},
            {"(echo -n  'Hello World!')", "Hello World!"},
            {"{echo -n  'Hello World!'}", "Hello World!"},
        };

        inullstream oStdIn;
        onullstream oStdErr;

        for (const auto &[sCommand, sOutput]: vTests) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, oStdErr);
            shell::run(sCommand, oSession);
            const std::string sName = "Check command " + sCommand + " - <" + oStdOut.str() + ">";
            custom_assert(sOutput == oStdOut.view(), sName);
        }
    }

    void test_shell::test_echo_envvar() const {
        const std::vector<std::pair<std::string, std::string> > vTests = {
            {"echo -n $pos1 $pos2", "env1 var2"},
            {"echo -n ${pos1} ${pos2}", "env1 var2"},
            {R"(echo -n "$pos1" "$pos2")", "env1 var2"},
            {R"(echo -n "${pos1}" "${pos2}")", "env1 var2"},
            {R"(echo -n "$pos1   $pos2")", "env1   var2"},
            {R"(echo -n "${pos1}   ${pos2}")", "env1   var2"},
            {"echo -n $(getenv pos1) $(getvar pos2)", "env1 var2"},
            {"echo -n \"$(getenv pos1)   $(getvar pos2)\"", "env1   var2"},
            {"echo -n `getenv pos1` `getvar pos2`", "env1 var2"},
            {"echo -n \"`getenv pos1`   `getvar pos2`\"", "env1   var2"},
            {"echo -n '$(getenv pos1)   $(getvar pos2)'", "$(getenv pos1)   $(getvar pos2)"},
            {"echo -n $(getenv pos1); echo -n $(getvar pos2)", "env1var2"},
            {"echo -n `getenv pos1`; echo -n `getvar pos2`", "env1var2"},
        };

        inullstream oStdIn;
        onullstream oStdErr;

        for (const auto &[sCommand, sOutput]: vTests) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, oStdErr);
            oSession.set_env("pos1", "env1");
            oSession.set_env("pos2", "env2");
            oSession.set_var("pos2", "var2");
            shell::run(sCommand, oSession);
            const std::string sName = "Check command " + sCommand + " - <" + oStdOut.str() + ">";
            custom_assert(sOutput == oStdOut.view(), sName);
        }
    }

    void test_shell::test_seq() const {
        const std::vector<std::pair<std::string, std::string> > vTests = {
            {"seq 1 5", "1 2 3 4 5"},
            {"seq 1 2 5", "1 3 5"},
            {"seq 5 -2 1", "5 3 1"},
            {"seq 5 1", "5 4 3 2 1"},
            {"echo -n $(seq 1 5)", "1 2 3 4 5"},
        };

        inullstream oStdIn;
        onullstream oStdErr;

        for (const auto &[sCommand, sOutput]: vTests) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, std::cout);
            shell::run(sCommand, oSession);
            const std::string sName = "Check command " + sCommand + " " + oStdOut.str();
            custom_assert(sOutput == oStdOut.view(), sName);
        }
    }

    void test_shell::test_setenv() const {
        inullstream oStdIn;
        std::ostringstream oStdOut;
        std::ostringstream oStdErr;
        shell_session oSession(m_pShell.get(), oStdIn, oStdOut, oStdErr);

        // Normal setenv
        shell_status res1 = shell::run("setenv variable value"sv, oSession);
        assert(res1 == shell_status::SHELL_SUCCESS);
        assert(oSession.get_env("variable")=="value");

        // Wrong setenv
        shell_status res2 = shell::run("setenv 1234 value"sv, oSession);
        assert(res2 == shell_status::SHELL_CMD_ERROR_SETENV_VARIABLE_NAME_INVALID);

        // Normal getenv
        shell_status res3 = shell::run("getenv variable"sv, oSession);
        assert(res3 == shell_status::SHELL_SUCCESS);
        assert(oStdOut.str()=="value");

        // Wrong setenv
        shell_status res4 = shell::run("getenv 1234"sv, oSession);
        assert(res4 == shell_status::SHELL_CMD_ERROR_GETENV_VARIABLE_NAME_INVALID);
    }

    void test_shell::test_setvar() const {
        inullstream oStdIn;
        std::ostringstream oStdOut;
        std::ostringstream oStdErr;
        shell_session oSession(m_pShell.get(), oStdIn, oStdOut, oStdErr);

        // Normal setenv
        shell_status res1 = shell::run("setvar variable value"sv, oSession);
        assert(res1 == shell_status::SHELL_SUCCESS);
        assert(oSession.get_var("variable")=="value");

        // Wrong setenv
        shell_status res2 = shell::run("setenv 1234 value"sv, oSession);
        assert(res2 == shell_status::SHELL_CMD_ERROR_SETENV_VARIABLE_NAME_INVALID);

        // Normal getvar
        shell_status res3 = shell::run("getvar variable"sv, oSession);
        assert(res3 == shell_status::SHELL_SUCCESS);
        assert(oStdOut.str()=="value");

        // Wrong setvar
        shell_status res4 = shell::run("getvar 1234"sv, oSession);
        assert(res4 == shell_status::SHELL_CMD_ERROR_GETVAR_VARIABLE_NAME_INVALID);
    }

    void test_shell::test_oper() const {
        const std::vector<std::pair<std::string, std::string> > vTests = {
            {"echo -n a && echo -n b", "ab"},
            {"echo -n a || echo -n b", "a"},
            {"echo -n a && echo -n b || echo -n c && echo -n d", "ab"},
            {"echo -n a || echo -n b && echo -n c || echo -n d", "a"},
            {"echo -n a && echo -n b ; echo -n c && echo -n d", "abcd"},
            {"echo -n a || echo -n b ; echo -n c || echo -n d", "ac"},
            {"(echo -n a || echo -n b) && (echo -n c || echo -n d)", "ac"},
            {"{echo -n a || echo -n b} && {echo -n c || echo -n d}", "ac"},
            {"(echo -n a || echo -n b) || (echo -n c || echo -n d)", "a"},
            {"{echo -n a || echo -n b} || {echo -n c || echo -n d}", "a"},
            {"(echo -n a && echo -n b) && (echo -n c && echo -n d)", "abcd"},
            {"{echo -n a && echo -n b} && {echo -n c && echo -n d}", "abcd"},
        };

        inullstream oStdIn;
        onullstream oStdErr;

        for (const auto &[sCommand, sOutput]: vTests) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, std::cout);
            shell::run(sCommand, oSession);
            const std::string sName = "Check command " + sCommand + " " + oStdOut.str();
            custom_assert(sOutput == oStdOut.view(), sName);
        }
    }

    void test_shell::test_structure() const {
        const std::vector<std::pair<std::string, std::string> > vTests = {
            {"for num in $(seq 1 5);do echo -n $num; done", "12345"},
            {"for   num   in $(seq 1 5)   ;  do   echo -n $num;   done", "12345"},
            {"for num in $(seq 1 5);do echo -n $num; echo -n $num; done", "1122334455"},
            {"for num in $(seq 1 5);do echo -n $num; continue; echo -n $num; done", "12345"},
            {"for num in $(seq 1 5);do echo -n $num; break; echo -n $num; done", "1"},
            {"for   num   in   $(seq 1 5);   do echo -n $num;   continue   ; echo -n $num;   done", "12345"},
            {"for   num   in   $(seq 1 5);   do echo -n $num;   break      ; echo -n $num;   done", "1"},
            {"if [-z \"\"]; then echo -n true; fi", "true"},
            {"if [ ( -z \"\" ) && ( -n \"d\" ) ]; then echo -n true; fi", "true"},
            {"if [ ( -z \"\" ) ] && [ ( -n \"d\" ) ]; then echo -n true; fi", "true"},
            {"if [-n \"\"]; then else echo -n true; fi", "true"},
            {"if [-n \"\"]; then elif [-z \"\"]; then echo -n true; fi", "true"},
            {"if [-n \"\"]; then elif [-n \"\"]; then else echo -n true; fi", "true"},
            {"while [ -n \"\" ]; do done ", ""},
            {"until [ -z \"\" ]; do done ", ""},
            {"while [ -z \"$stop\" ]; do setvar stop stop; echo -n stop; done ", "stop"},
            {"until [ -n \"$stop\" ]; do setvar stop stop; echo -n stop; done ", "stop"},
            {"function ignore{} fcall ignore", ""},
            {"function echon {echo -n \"$1\"} fcall echon 'Hello World!'", "Hello World!"},
            {"function echon {echo -n $@} fcall echon Hello World!", "Hello World!"},
        };

        inullstream oStdIn;
        onullstream oStdErr;

        for (const auto &[sCommand, sOutput]: vTests) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, std::cout);
            shell::run(sCommand, oSession);
            const std::string sName = "Check command " + sCommand + " : " + oStdOut.str();
            custom_assert(sOutput == oStdOut.view(), sName);
        }
    }

    void test_shell::test_math() const {
        const std::vector<std::pair<std::string, std::string> > vTests = {
            {"math + 1", "1"},
            {"math - 1", "-1"},
            {"math +1", "1"},
            {"math -1", "-1"},
            {"math 3 + 4", "7"},
            {"math 3 * 4", "12"},
            {"math 12 / 4", "3"},
            {"math 12 % 5", "2"},
            {"math 2 ^ 3", "8"},
            {"math 2 ** 3", "8"},
            {"math 2 ** - 3", "0"},
            {"math 2 + 2 * 2 + 2 ^ 2 + 2 * 2 + 2", "16"},
            {"math 42 ^ 0 + 1 ^ 42 + 0 ^ 42", "2"},
            {"math \\( 2 + 2 \\) * \\( 2 + 2 \\) ^ \\( 2 + 2 \\) * \\( 2 + 2 \\)", "4096"},
            {"math $(echo \"( 2 + 2 ) * ( 2 + 2 ) ^ ( 2 + 2 ) * ( 2 + 2 )\" )", "4096"},
            {"math \\( \\( 2 + 2 \\) * \\( 2 + 2 \\) \\) ^ \\( \\( 1 + 2 \\) * \\( 1 + 2 \\) \\)", "68719476736"},
            {"math factorial \\( 5 \\)", "120"},
            {"math product \\( x , 1 , 1 , 5 , x \\)", "120"},
            {"math sum \\( x , 1 , 1 , 5 , x \\)", "15"},
            {"math sign \\( - 42 \\) ; math sign \\( 0 \\) ; math sign \\( + 42 \\)", "-101"},
            {"math abs \\( - 42 \\) ; math abs \\( + 42 \\)", "4242"},
            {"math sum \\( x , 1 , 1 , 5 , sum \\( x , 1 , 1 , 3 , x \\) \\)", "30"},
            {"math sum \\( x , 1 , 1 , 5 , sum \\( y , 1 , 1 , 3 , x \\) \\)", "45"},
        };

        inullstream oStdIn;
        onullstream oStdErr;

        for (const auto &[sCommand, sOutput]: vTests) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, oStdErr);
            shell::run(sCommand, oSession);
            const std::string sName = "Check command " + sCommand + " " + oStdOut.str();
            custom_assert(sOutput == oStdOut.view(), sName);
        }

        std::vector<std::string> vFails = {
            {"math +"},
            {"math 5 +"},
            {"math 5+5"},
            {"math \\( 5 + 5"},
            {"math 1 / 0"},
            {"math 1 % 0"},
            {"math 0 ^ 0"},
            {"math sum \\( x , 1 , - 1 , 5 , x \\)"},
            {"math product \\( x , 1 , - 1 , 5 , x \\)"},
            {"math sum \\( x , 1 , 0 , 5 , x \\)"},
            {"math product \\( x , 1 , 0 , 5 , x \\)"},
            {"math sum \\( x , 1 , 1 , - 5 , x \\)"},
            {"math product \\( x , 1 , 1 , - 5 , x \\)"},
            {"math sum \\( x , 1 , 0 , - 5 , x \\)"},
            {"math product \\( x , 1 , 0 , - 5 , x \\)"},
            {"math sum \\( x , 1 , 1 , 5 , y \\)"},
            {"math product \\( x , 1 , 1 , 5 , y \\)"},
            {"math abs \\( x , 1 \\)"},
        };

        for (const auto &sCommand: vFails) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, std::cout);
            const auto nStatus = shell::run(sCommand, oSession);
            assert(nStatus!=shell_status::SHELL_SUCCESS);
        }
    }

    void test_shell::test_test() const {
        const std::vector<std::pair<std::string, bool> > vTests = {
            {"test -z \"\"", true},
            {"test -z \"d\"", false},
            {"test -n \"\"", false},
            {"test -n \"d\"", true},
            {"test -z \"\" -a -n \"d\"", true},
            {"test -z \"\" && -n \"d\"", true},
            {"test -z \"\" -o -z \"d\"", true},
            {"test -n \"\" && -n \"d\"", false},
            {"test \\( -n d \\) && \\( -z \"\" \\)", true},
            {"test -z -eq", false},
            {"test 7 -eq 0007", true},
            {"test 7 == 0007", true},
            {"test 7 == 42", false},
            {"test abc -eq abc", true},
            {"test abc == abc", true},
            {"test abc == abcd", false},
            {"test 7 -ne 0007", false},
            {"test 7 != 0007", false},
            {"test 7 != 42", true},
            {"test abc -ne abc", false},
            {"test abc != abc", false},
            {"test abc != abcd", true},
            {"test 7 -gt 6", true},
            {"test 7 -gt 7", false},
            {"test 6 -gt 7", false},
            {"test 7 > 6", true},
            {"test 7 > 7", false},
            {"test 6 > 7", false},
            {"test b -gt a", true},
            {"test b -gt b", false},
            {"test a -gt b", false},
            {"test b > a", true},
            {"test b > b", false},
            {"test a > b", false},
            {"test 6 -lt 7", true},
            {"test 7 -lt 7", false},
            {"test 8 -lt 7", false},
            {"test 7 < 7", false},
            {"test 6 < 7", true},
            {"test 8 < 7", false},
            {"test a -lt b", true},
            {"test b -lt a", false},
            {"test b -lt b", false},
            {"test a < b", true},
            {"test b < a", false},
            {"test b < b", false},
            {"test 6 -ge 7", false},
            {"test 7 -ge 7", true},
            {"test 8 -ge 7", true},
            {"test 7 >= 7", true},
            {"test 6 >= 7", false},
            {"test 8 >= 7", true},
            {"test a -ge b", false},
            {"test b -ge a", true},
            {"test b -ge b", true},
            {"test a >= b", false},
            {"test b >= a", true},
            {"test b >= b", true},
            {"test 6 -le 7", true},
            {"test 7 -le 7", true},
            {"test 8 -le 7", false},
            {"test 7 <= 7", true},
            {"test 6 <= 7", true},
            {"test 8 <= 7", false},
            {"test a -le b", true},
            {"test b -le a", false},
            {"test b -le b", true},
            {"test a <= b", true},
            {"test b <= a", false},
            {"test b <= b", true},
            {"test 'hello' =~ '^h.*o$'", true},
            {"test 'hello' =~ '^h.*l$'", false},
            {"test 'hello' =~ 'world'", false},
            {"test '12345' =~ '^[0-9]+$'", true},
            {"test 'abc123' =~ '^[a-zA-Z]+[0-9]+$'", true},
            {"test 'abc' =~ '^[0-9]+$'", false},
            {"test 'test@example.com' =~ '^[^@]+@[^@]+\\\\.[^@]+$'", true},
            {"test 'invalid-email' =~ '^[^@]+@[^@]+\\\\.[^@]+$'", false},
            {"test 'C++ Programming' =~ '.*C\\\\+\\\\+.*'", true},
            {"test 'C Programming' =~ '.*C\\\\+\\\\+.*'", false},
            {"test 'C++' =~ '\\\\+\\\\+'", false},
            {"test 'C#' =~ '.*\\\\+.*'", false},
            {"test 'C#' =~ '.*#+.*'", true},
        };

        inullstream oStdIn;
        onullstream oStdErr;
        onullstream oStdOut;
        shell_session oSession(m_pShell.get(), oStdIn, oStdOut, std::cout);

        for (const auto &[sCommand, bPass]: vTests) {
            const auto nStatus = shell::run(sCommand, oSession);
            const bool bResult = nStatus == shell_status::SHELL_SUCCESS;
            if (bResult != bPass) {
                std::cout << "Failed: " << sCommand << std::endl;
            }
            assert(bResult == bPass);
        }
    }

    void test_shell::test_script() const {
        const std::vector<std::pair<std::string, std::string> > vTests = {
            {"echo -n 'Hello, World!'", "Hello, World!"},
            {"function count_args { echo -n $# } fcall count_args $(seq 1 5)", "5"},
            {
                "setvar count 1; while [ $count <= 5 ]; do echo \"Count: $count\"; setvar count $(math $count + 1); done",
                "Count: 1\nCount: 2\nCount: 3\nCount: 4\nCount: 5\n"
            },
            {
                "setvar count 1; until [ $count > 5 ]; do echo \"Count: $count\"; setvar count $(math $count + 1); done",
                "Count: 1\nCount: 2\nCount: 3\nCount: 4\nCount: 5\n"
            },
            {
                "for i in $(seq 1 5); do echo -n $i; done",
                "12345"
            },
            {
                "function greet { echo \"Ave $1\" } fcall greet Cesar",
                "Ave Cesar\n"
            },
            {
                "function oddeven { if [ $(math $1 % 2) == 0 ]; then echo \"$1 is even\"; else echo \"$1 is odd\"; fi } fcall oddeven 42; fcall oddeven 11;",
                "42 is even\n11 is odd\n"
            },
            {
                "function show_args { if [ $# > 1 ]; then for i in $(seq 1 $#); do echo \"arg $i: \\u201C${!i}\\u201D\"; done else echo 'No arguments'; fi } fcall show_args; fcall show_args $(seq 1 5)",
                "No arguments\n"
                "arg 1: \u201C1\u201D\n"
                "arg 2: \u201C2\u201D\n"
                "arg 3: \u201C3\u201D\n"
                "arg 4: \u201C4\u201D\n"
                "arg 5: \u201C5\u201D\n"
            },
            {
                "setvar cmd 'echo -n 1; echo -n 2'; $cmd",
                "1; echo -n 2"
            },
        };

        inullstream oStdIn;
        onullstream oStdErr;

        for (const auto &[sCommand, sOutput]: vTests) {
            std::ostringstream oStdOut;
            shell_session oSession(m_pShell.get(), oStdIn, oStdOut, oStdErr);
            shell::run(sCommand, oSession);
            const std::string sName = "Check command " + sCommand + " " + oStdOut.str();
            custom_assert(sOutput == oStdOut.view(), sName);
        }
    }
}
