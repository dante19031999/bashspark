/**
 * @file shell_arg.h
 * @brief Argument list class for managing shell commands.
 *
 * Provides an interface to retrieve command-line arguments.
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

#include <memory>
#include <string>
#include <vector>

namespace bs {
    /**
     * @class shell_arg
     * @brief Represents a command line argument list for shell commands.
     *
     * This class manages command-line arguments.
     */
    class shell_arg {
    public:
        /// Default constructor (no args)
        shell_arg() = default;

        /**
         * @brief Constructs an shell_arg object with command-line arguments.
         * @param vArgVariables A vector containing command-line arguments.
         */
        explicit shell_arg(std::vector<std::string> &&vArgVariables)
            : m_vArgVariables(std::move(vArgVariables)) {
        }

    public:
        /**
         * @brief Retrieves a command-line argument by its index.
         * @param nArg The index of the argument.
         * @return The argument as a string, or an empty string if out of range.
         */
        [[nodiscard]] std::string get_arg(const std::size_t nArg) const {
            return nArg < this->m_vArgVariables.size() ? m_vArgVariables.at(nArg) : "";
        }

        /**
         * @brief Checks if a command-line argument exists at the given index.
         * @param nArg The index of the argument.
         * @return True if the argument exists, false otherwise.
         */
        [[nodiscard]] bool has_arg(const std::size_t nArg) const noexcept {
            return nArg < this->m_vArgVariables.size();
        }

        /**
         * @brief Gets the number of command-line arguments.
         * @return The size of the arguments vector.
         */
        [[nodiscard]] std::size_t get_arg_size() const noexcept {
            return this->m_vArgVariables.size();
        }

        /**
         * @brief Retrieves all command-line arguments.
         * @return A const reference to the vector of command-line arguments.
         */
        [[nodiscard]] const std::vector<std::string> &get_args() const noexcept {
            return this->m_vArgVariables;
        }

    private:
        /// Vector to store command-line arguments.
        std::vector<std::string> m_vArgVariables;
    };
} // namespace bs
