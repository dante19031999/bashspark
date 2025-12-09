/**
 * @file command_math.h
 * @brief Defines command `bs::command_math`.
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
     *
     * @class command_math
     * @brief Prints on stdout the result of a mathematical operation
     *
     * Syntax: math mathematics expression
     *
     * Supports:
     * - Operators: + - * / ^ ** × ÷
     * - Integer numbers within the range of bc2 64 bits
     * - Parentheses: ( ) (and nested parentheses).
     * - Negative powers partially: { x != 0 } ^ { y < 0 } = 0
     * - Functions:
     *   - factorial(x)
     *   - abs(x): |x|
     *   - sign(x): x != 0: x / abs(x), x == 0: 0
     *   - sum(variable; start; end; step; expression) where expression is f(variable), ex: count = sum(x;1;1;100;1)
     *   - product(variable; start; end; step; expression) where expression is f(variable), ex: factorial = product(x;1;1;100;x)
     *   - Note that sum/product with same variable, the variable from the one inside will hide the variable of the one outside
     *   - Note that sum/product require variable names acceptable by the shell
     *
     * Possible errors:
     *   - Not an integer: `bs::shell_status::SHELL_CMD_ERROR_MATH_NOT_AN_INTEGER`
     *   - Overflow: `bs::shell_status::SHELL_CMD_ERROR_MATH_OVERFLOW`
     *   - Underflow: `bs::shell_status::SHELL_CMD_ERROR_MATH_UNDERFLOW`
     *   - Division by 0: `bs::shell_status::SHELL_CMD_ERROR_MATH_DIV_BY_ZERO`
     *   - Power of 0^0: `bs::shell_status::SHELL_CMD_ERROR_MATH_POW_0_EXP_0`
     *   - Factorial of negative: `bs::shell_status::SHELL_CMD_ERROR_MATH_FACTORIAL_NEGATIVE`
     *   - Malformed expression: `bs::shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION`
     *   - Maximum depth reached (512 as of now): `bs::shell_status::SHELL_CMD_ERROR_MATH_MAX_DEPTH_REACHED`
     *   - Invalid variable name (on function variable): `bs::shell_status::SHELL_CMD_ERROR_MATH_INVALID_VARIABLE_NAME`
     *   - Sequence logic error if sequence function is invalid: `bs::shell_status::SHELL_CMD_ERROR_MATH_SEQ_ITERATION_LOGIC`
     *
     */
    class command_math : public command {
    public:
        /**
         * @brief Constructs command
         */
        command_math()
            : command("math") {
        }

    public:
        /**
         *
         * @brief Prints on stdout the result of a mathematical operation
         * @param vArgs Arguments for the command.
         * @param oSession The shell session context.
         * @return Status of command execution.
         */
        shell_status run(const std::span<const std::string> &vArgs, shell_session &oSession) const override;

    public:
        /**
         * @brief Displays the error message for math errors
         *
         * Can be overwritten with custom behaviour.
         *
         * @param oStdErr Error output stream
         * @param nStatus Status code
         */
        virtual void msg_error_math(std::ostream &oStdErr, shell_status nStatus) const;
    };
}
