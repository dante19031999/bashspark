/**
 * @file shell_status.h
 * @brief Defines status codes for the shell.
 *
 * This header file contains constants that represent various shell status codes.
 * These codes indicate the result of executing commands or operations within the shell.
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
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cinttypes>
#include <iostream>

namespace bs {
    /**
     * @brief Maximum depth allowed for shell
     *
     * Surpassing this depth will result in controlled errors.
     */
    constexpr std::size_t SHELL_MAX_DEPTH = 16;

    /**
     * @brief Shell status codes
     * @warning Defining command error codes below `bs::shell_status::SHELL_CMD_ERROR` results in undefined behaviour.
     */
    enum class shell_status : std::uint32_t {
        // @section shell Execution Status Codes

        /// Indicates successful execution.
        SHELL_SUCCESS = 0,

        /// Indicates a generic error occurred.
        SHELL_ERROR = 1,

        /// Indicates a syntax error in the command.
        SHELL_ERROR_SYNTAX_ERROR,

        /// Indicates a syntax error in the command, unclosed simple quotes.
        SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SIMPLE_QUOTES,

        /// Indicates a syntax error in the command, unclosed double quotes.
        SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_DOUBLE_QUOTES,

        /// Indicates a syntax error in the command, unclosed back quotes.
        SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_BACK_QUOTES,

        /// Indicates a syntax error in the command, unclosed parentheses.
        SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_PARENTHESES,

        /// Indicates a syntax error in the command, unclosed brackets.
        SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_BRACKETS,

        /// Indicates a syntax error in the command, unclosed square brackets.
        SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SQR_BRACKETS,

        /// Indicates a syntax error in the command, unclosed back quotes.
        SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SUBCOMMAND,

        /// Indicates a syntax error in the command, unclosed back variable.
        SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_VARIABLE,

        /// Indicates a syntax error in the command, invalid variable name.
        SHELL_ERROR_SYNTAX_ERROR_INVALID_VARIABLE_NAME,

        /// Indicates a syntax error in the command, unexpected token.
        SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,

        /// Indicates a syntax error in the command, unexpected eof.
        SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_EOF,

        /// Indicates a syntax error in the command, an argument out of range.
        SHELL_ERROR_SYNTAX_ERROR_ARG_OUT_OF_RANGE,

        /// Indicates a syntax error in the command, an empty block.
        SHELL_ERROR_SYNTAX_ERROR_EMPTY_BLOCK,

        /// Indicates a syntax error in the command, the keyword "if" is not finished
        SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_IF,

        /// Indicates a syntax error in the command, the keyword "then" is missing
        SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_THEN,

        /// Indicates a syntax error in the command, the keyword "loop" is not finished
        SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_LOOP,

        /// Indicates a syntax error in the command, the keyword "for" is not finished
        SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_FOR,

        /// Indicates a syntax error in the command, the keyword "in" is missing
        SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_IN,

        /// Indicates a syntax error in the command, the keyword "while" is not finished
        SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_WHILE,

        /// Indicates a syntax error in the command, the keyword "until" is not finished
        SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_UNTIL,

        /// Indicates a syntax error in the command, the keyword "do" is missing
        SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_DO,

        /// Indicates a syntax error in the command, invalid function name.
        SHELL_ERROR_SYNTAX_ERROR_INVALID_FUNCTION_NAME,

        /// Indicates a syntax error in the command, the function body definition is wrong.
        SHELL_ERROR_SYNTAX_ERROR_INVALID_FUNCTION_BODY,

        /// Indicates a syntax error in encoding.
        SHELL_ERROR_BAD_ENCODING,

        /// Indicates the command was not found.
        SHELL_ERROR_COMMAND_NOT_FOUND,

        /// Indicates the maximum depth for command nesting has been reached.
        SHELL_ERROR_MAX_DEPTH_REACHED,

        // @section command Command Errors

        /**
         * @brief Indicates base error value for commands
         *
         * @warning Defining command error codes below this value results in undefined behaviour.
         */
        SHELL_CMD_ERROR = 42,

        // @section envvar Command getenv, getvar, setenv, setvar errors

        /// Indicates an error with the number of parameters for command getenv
        SHELL_CMD_ERROR_GETENV_PARAM_NUMBER = SHELL_CMD_ERROR + 1,

        /// Indicates that the variable name passed to getenv is not valid
        SHELL_CMD_ERROR_GETENV_VARIABLE_NAME_INVALID,

        /// Indicates an error with the number of parameters for command getenv
        SHELL_CMD_ERROR_GETVAR_PARAM_NUMBER = SHELL_CMD_ERROR + 1,

        /// Indicates that the variable name passed to getenv is not valid
        SHELL_CMD_ERROR_GETVAR_VARIABLE_NAME_INVALID,

        /// Indicates an error with the number of parameters for command setenv
        SHELL_CMD_ERROR_SETENV_PARAM_NUMBER = SHELL_CMD_ERROR + 1,

        /// Indicates that the variable name passed to setenv is not valid
        SHELL_CMD_ERROR_SETENV_VARIABLE_NAME_INVALID,

        /// Indicates an error with the number of parameters for command setenv
        SHELL_CMD_ERROR_SETVAR_PARAM_NUMBER = SHELL_CMD_ERROR + 1,

        /// Indicates that the variable name passed to setenv is not valid
        SHELL_CMD_ERROR_SETVAR_VARIABLE_NAME_INVALID,

        // @section seq Command seq errors

        /// Indicates an error with the number of parameters for command seq
        SHELL_CMD_ERROR_SEQ_PARAM_NUMBER = SHELL_CMD_ERROR + 1,

        /// Indicates that the parameter seq received is not a number
        SHELL_CMD_ERROR_SEQ_INVALID_INT_FORMAT,

        /// Indicates that the parameter seq received a number out of bounds
        SHELL_CMD_ERROR_SEQ_INT_OUT_OF_BOUNDS,

        /// Command seq: error indicating that the sequence can not be iterated
        SHELL_CMD_ERROR_SEQ_ITERATION_LOGIC,

        // @section test Command test errors

        /// Command test: error indicating that opened parenthesis have not been closed
        SHELL_CMD_ERROR_TEST_UNCLOSED_PARENTHESIS = SHELL_CMD_ERROR + 1,

        /// Command test: error indicating that the expression is malformed
        SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION,

        /// Command test: error indicating that a regex comparation is using an invalid regex
        SHELL_CMD_ERROR_TEST_MALFORMED_REGEX,

        /// Command test: status code indicating that the test is false
        SHELL_CMD_TEST_FALSE,

        // @section math Command math errors

        /// Command math: error indicating that the input is not an integer.
        SHELL_CMD_ERROR_MATH_NOT_AN_INTEGER = SHELL_CMD_ERROR + 1,

        /// Command math: error indicating that the result of a mathematical operation has exceeded the maximum limit.
        SHELL_CMD_ERROR_MATH_OVERFLOW,

        /// Command math: error indicating that the result of a mathematical operation has fallen below the minimum limit.
        SHELL_CMD_ERROR_MATH_UNDERFLOW,

        /// Command math: error indicating an attempt to divide by zero, which is undefined.
        SHELL_CMD_ERROR_MATH_DIV_BY_ZERO,

        /// Command math: error indicating that the calculation of 0 raised to the power of 0 is not defined.
        SHELL_CMD_ERROR_MATH_POW_0_EXP_0,

        /// Command math: error indicating an attempt to compute the factorial of a negative number, which is not possible.
        SHELL_CMD_ERROR_MATH_FACTORIAL_NEGATIVE,

        /// Command math: error indicating that the mathematical expression provided is improperly formatted.
        SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION,

        /// Command math: error indicating that the maximum recursion or computation depth has been reached.
        SHELL_CMD_ERROR_MATH_MAX_DEPTH_REACHED,

        /// Command math: error indicating that the specified variable name does not conform to valid naming conventions.
        SHELL_CMD_ERROR_MATH_INVALID_VARIABLE_NAME,

        /// Command math: error indicating that the sequence can not be iterated
        SHELL_CMD_ERROR_MATH_SEQ_ITERATION_LOGIC,

        // @section fcall Command fcall errors

        /// Indicates an error with the number of parameters for command fcall
        SHELL_CMD_ERROR_FCALL_PARAM_NUMBER = SHELL_CMD_ERROR + 1,

        /// Command fcall: Indicates the funcion was not found.
        SHELL_CMD_ERROR_FCALL_FUNCTION_NOT_FOUND,
    };

    /**
     * @brief Generates status codes for user use from user codes
     * @param nCode User code
     * @return Generated status code
     */
    constexpr shell_status make_user_code(const unsigned int nCode) {
        return static_cast<shell_status>(static_cast<int>(shell_status::SHELL_CMD_ERROR) + nCode);
    }

    /**
     * Check whether a status code represents a syntax error
     * @param nStatus Status code to check
     * @return If the status code represents to a syntax error true, false otherwhise
     */
    constexpr bool is_syntax_error(const shell_status nStatus) {
        return nStatus >= shell_status::SHELL_ERROR_SYNTAX_ERROR
               && nStatus <= shell_status::SHELL_ERROR_MAX_DEPTH_REACHED;
    }
}
