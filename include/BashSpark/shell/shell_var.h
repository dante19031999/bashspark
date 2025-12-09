/**
 * @file shell_var.h
 * @brief Variable class for managing shell commands.
 *
 * Provides an interface to retrieve and set variables
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

#include <string>

#include "BashSpark/tools/shell_hash.h"

namespace bs {
    /**
     * @class shell_var
     * @brief Represents a variable map for shell commands.
     *
     * This class manages variables.
     */
    class shell_var {
    public:
        /// Default constructor.
        shell_var() = default;

    public:
        /**
         * @brief Retrieves the value of a variable.
         * @param sVar The name of the variable.
         * @return The value of the variable, or an empty string if not found.
         */
        [[nodiscard]] std::string get_var(const std::string &sVar) const {
            const auto pIter = this->m_mVariables.find(sVar);
            return pIter != this->m_mVariables.end() ? pIter->second : "";
        }

        /**
         * @brief Retrieves the value of a variable,
         * whose value is another variable (1-hop resolution).
         * @param sVar The name of the variable.
         * @return The value of the resolved variable, or an empty string if not found.
         */
        [[nodiscard]] std::string get_var_hop2(const std::string &sVar) const {
            const auto pIter1 = this->m_mVariables.find(sVar);
            if (pIter1 == this->m_mVariables.end()) return "";
            const auto pIter2 = this->m_mVariables.find(pIter1->second);
            return pIter2 != this->m_mVariables.end() ? pIter2->second : "";
        }

        /**
         * @brief Sets the value of a variable.
         * @param sVar The name of the variable.
         * @param sValue The value to be set.
         */
        void set_var(std::string sVar, std::string sValue) {
            this->m_mVariables.insert_or_assign(std::move(sVar), std::move(sValue));
        }

        /**
         * @brief Checks if a variable exists.
         * @param sVar The name of the variable.
         * @return True if the variable exists, false otherwise.
         */
        [[nodiscard]] bool has_var(const std::string &sVar) const noexcept {
            return this->m_mVariables.contains(sVar);
        }

        /**
         * @brief Gets the number of variables.
         * @return The size of the variables map.
         */
        [[nodiscard]] std::size_t get_var_size() const noexcept {
            return this->m_mVariables.size();
        }

        /**
         * @brief Retrieves all variables.
         * @return A const reference to the map of variables.
         */
        [[nodiscard]] const std::unordered_map<std::string, std::string, shell_hash> &get_var() const noexcept {
            return this->m_mVariables;
        }

    private:
        /// Map to store variables.
        std::unordered_map<std::string, std::string, shell_hash> m_mVariables;

    };
} // namespace bs
