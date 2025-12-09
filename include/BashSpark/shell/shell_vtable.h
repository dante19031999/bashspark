/**
 * @file shell_vtable.h
 * @brief Function table class for managing shell commands.
 *
 * Provides an interface to manage shell session functions
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
#include <unordered_map>

#include "shell_node.h"
#include "BashSpark/tools/shell_hash.h"

namespace bs {
    class shell_node_evaluable;
}

namespace bs {
    /**
     * @class shell_vtable
     * @brief Represents an environment for shell commands.
     *
     * This class manages environment variables.
     */
    class shell_vtable {
    public:
        /// Sell function type
        using func_type = shell_node_evaluable;

    public:
        /// Default constructor (empty environment).
        shell_vtable() = default;

    public:
        /**
         * @brief Gets a function
         * @param sVar The name of the function to get
         * @return The function if there is or nullptr
         */
        [[nodiscard]] const func_type* get_func(const std::string &sVar) const {
            const auto pIter = this->m_mFunctions.find(sVar);
            return pIter != this->m_mFunctions.end() ? pIter->second : nullptr;
        }

        /**
         * @brief Sets the a function.
         * @param sName The name of the function.
         * @param pFunction The function to be set.
         */
        void set_func(const std::string& sName, const func_type* pFunction) {
            this->m_mFunctions[sName] =  pFunction;
        }

        /**
         * @brief Checks if a function exists.
         * @param sName The name of the function.
         * @return True if the function exists, false otherwise.
         */
        [[nodiscard]] bool has_func(const std::string &sName) const noexcept {
            return this->m_mFunctions.contains(sName);
        }

        /**
         * @brief Gets the number of functions in the vtable.
         * @return The size of the function map.
         */
        [[nodiscard]] std::size_t get_vtable_size() const noexcept {
            return this->m_mFunctions.size();
        }

        /**
         * @brief Retrieves all environment variables.
         * @return A const reference to the map of environment variables.
         */
        [[nodiscard]] const std::unordered_map<std::string, const func_type*, shell_hash> &get_env() const noexcept {
            return this->m_mFunctions;
        }

    private:
        /// Map to store environment variables.
        std::unordered_map<std::string, const func_type*, shell_hash> m_mFunctions;
    };
} // namespace bs
