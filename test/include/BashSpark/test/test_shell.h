/**
 * @file test_shell.h
 * @brief Defines a series of tests to check the overall compliance of the shell.
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

#pragma once

#include "BashSpark/BashSpark.h"

namespace bs::debug {
    /**
     * @class test_shell
     * @brief A class to encapsulate shell testing functionalities.
     *
     * This class provides methods to test shell commands and
     * environment variable handling within the BashSpark framework.
     */
    class test_shell {
    public:
        /**
         * @brief Constructs a test_shell object.
         *
         * Initializes the shell pointer.
         */
        test_shell();

        /**
         * @brief Executes a series of tests.
         *
         * This method will call various test methods within the class
         * to verify shell functionalities.
         */
        void test() const;

    private:
        /**
         * @brief Tests basic functionality.
         *
         * This method verifies the simplest cases  in the shell.
         */
        void test_basic() const;

        /**
         * @brief Tests basic echo functionality.
         *
         * This method verifies the simplest case of echoing text in the shell.
         */
        void test_echo_basic() const;

        /**
         * @brief Tests environment variable echo functionality.
         *
         * This method checks if environment variables can be echoed correctly.
         */
        void test_echo_envvar() const;

        /**
         * @brief Tests sequential command execution.
         *
         * Ensures that commands can be executed in a sequence as expected.
         */
        void test_seq() const;

        /**
         * @brief Tests setting an environment variable.
         *
         * This method verifies that environment variables can be set correctly.
         */
        void test_setenv() const;

        /**
         * @brief Tests setting a shell variable.
         *
         * This method checks if shell variables can be set within the shell.
         */
        void test_setvar() const;

        /**
         * @brief Tests basic operator use.
         *
         * This method verifies the simplest operators uses in the shell.
         */
        void test_oper() const;

        /**
         * @brief Tests basic structure use.
         *
         * This method verifies the simplest operators uses in the shell.
         */
        void test_structure()const;

        /**
         * @brief Tests basic command math use.
         *
         * This method verifies the simplest operators uses in the shell.
         */
        void test_math()const;

        /**
         * @brief Tests basic command test use.
         *
         * This method verifies the simplest operators uses in the shell.
         */
        void test_test()const;

        /**
         * @brief Tests some simple scripts
         *
         * This method verifies the simplest scripts in the shell.
         */
        void test_script()const;

    private:
        std::unique_ptr<shell> m_pShell; ///< Pointer to the shell instance used for testing.
    };
}
