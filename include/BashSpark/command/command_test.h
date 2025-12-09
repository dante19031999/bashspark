/**
 * @file command_test.h
 * @brief Defines command `bs::command_test`.
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

#include "BashSpark/command.h"

namespace bs {
    /**
     * @class command_test
     * @brief Performs a test. Returns through status code.
     *
     * Syntax: test args
     * Syntax: test args
     *
     * Capabilities:
     * - Operators or: -o, ||
     * - Operators and: -a, &&
     * - Comparison operators: ==, -eq, >, -gt, <, -lt, >=, -ge, <=, -le
     *   Note that comparison operators will apply numeric comparison if both arguments are
     *   numbers or <=> on std::string if they aren't.
     * - Operator =~: use a =~ b, checks if a matches b. Uses C++ regex, not bash regex.
     * - Parentheses: ( ) (and nested parentheses).
     *
     * Possible outputs:
     * - Test passed: `bs::shell_status::SHELL_SUCCESS`
     * - Test failed: `bs::shell_status::SHELL_CMD_TEST_FALSE`
     * - Opened parenthesis not closed: `bs::shell_status::SHELL_CMD_ERROR_TEST_UNCLOSED_PARENTHESIS`
     * - Malformed expression: `bs::shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION`
     * - Malformed regex: `bs::shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_REGEX`
     *
     */
    class command_test : public command {
    public:
        /**
         * @brief Constructs command
         */
        command_test()
            : command("test") {
        }

    public:
        /**
        * @brief Performs a test
        *
        * @param vArgs Arguments for the command.
        * @param oSession The shell session context.
        * @return Status of command execution.
        */
        [[nodiscard]] shell_status run(
            const std::span<const std::string> &vArgs,
            shell_session &oSession
        ) const override;

    public:
        /**
         * @brief Displays the error message for test errors
         *
         * Can be overwritten with custom behaviour.
         *
         * @param oStdErr Error output stream
         * @param nStatus Status code
         */
        virtual void msg_error_test(std::ostream &oStdErr, shell_status nStatus) const;
    };
}
