/**
 * @file shell_parser_exception.h
 * @brief Defines class shell_exception.
 *
 * This file contains the definition of the `shell_parser_exception` class,
 * which represents shell parsing errors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>

#include "BashSpark/shell/shell_status.h"
#include "BashSpark/tools/fakestream.h"

namespace bs {
    /**
     * @class shell_parser_exception
     * @brief Exception class for handling shell-specific errors.
     *
     * This exception class is used to capture and report errors encountered
     * during the parsing and execution of shell commands.
     */
    class shell_parser_exception final : public std::runtime_error {
    public:
        /**
         * @brief Constructs a shell_exception instance.
         * @param nStatus The status indicating the type of error.
         * @param sCommand The command that caused the error.
         * @param nPos The position in the command where the error occurred.
         */
        shell_parser_exception(
            shell_status nStatus,
            std::string sCommand,
            std::size_t nPos
        );

    public:
        /**
         * @brief Returns the error status associated with the exception.
         * @return The shell_status indicating the type of error.
         */
        [[nodiscard]] shell_status get_status() const noexcept {
            return m_nStatus;
        }

        /**
         * @brief Returns the command that caused the exception.
         * @return A string containing the command.
         */
        [[nodiscard]] const std::string &get_command() const noexcept {
            return m_sCommand;
        }

        /**
         * @brief Returns the position in the command where the error occurred.
         * @return The position as a size_t value.
         */
        [[nodiscard]] std::size_t get_position() const noexcept {
            return m_nPos;
        }

    private:
        /// The error status.
        shell_status m_nStatus;
        /// The command that caused the exception.
        std::string m_sCommand;
        /// The position of the error in the command.
        std::size_t m_nPos;
    };
}
