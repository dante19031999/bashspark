/**
 * @file shell_env.h
 * @brief Environment class for managing shell commands.
 *
 * Provides an interface to retrieve and set environment variables
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

#include <string>

#include "BashSpark/tools/shell_hash.h"

namespace bs {
    /**
     * @class shell_env
     * @brief Represents an environment for shell commands.
     *
     * This class manages environment variables.
     */
    class shell_env {
    public:
        /// Default constructor (empty environment).
        shell_env() = default;

    public:
        /**
         * @brief Retrieves the value of an environment variable.
         * @param sVar The name of the environment variable.
         * @return The value of the environment variable, or an empty string if not found.
         */
        [[nodiscard]] std::string get_env(const std::string &sVar) const {
            const auto pIter = this->m_mEnvVariables.find(sVar);
            return pIter != this->m_mEnvVariables.end() ? pIter->second : "";
        }

        /**
         * @brief Retrieves the value of an environment variable,
         * whose value is another variable (1-hop resolution).
         * @param sVar The name of the environment variable.
         * @return The value of the resolved environment variable, or an empty string if not found.
         */
        [[nodiscard]] std::string get_env_hop2(const std::string &sVar) const {
            const auto pIter1 = this->m_mEnvVariables.find(sVar);
            if (pIter1 == this->m_mEnvVariables.end()) return "";
            const auto pIter2 = this->m_mEnvVariables.find(pIter1->second);
            return pIter2 != this->m_mEnvVariables.end() ? pIter2->second : "";
        }

        /**
         * @brief Sets the value of an environment variable.
         * @param sVar The name of the environment variable.
         * @param sValue The value to be set.
         */
        void set_env(std::string sVar, std::string sValue) {
            this->m_mEnvVariables.insert_or_assign(std::move(sVar), std::move(sValue));
        }

        /**
         * @brief Checks if an environment variable exists.
         * @param sVar The name of the environment variable.
         * @return True if the variable exists, false otherwise.
         */
        [[nodiscard]] bool has_env(const std::string &sVar) const noexcept {
            return this->m_mEnvVariables.contains(sVar);
        }

        /**
         * @brief Gets the number of environment variables.
         * @return The size of the environment variables map.
         */
        [[nodiscard]] std::size_t get_env_size() const noexcept {
            return this->m_mEnvVariables.size();
        }

        /**
         * @brief Retrieves all environment variables.
         * @return A const reference to the map of environment variables.
         */
        [[nodiscard]] const std::unordered_map<std::string, std::string, shell_hash> &get_env() const noexcept {
            return this->m_mEnvVariables;
        }

    private:
        /// Map to store environment variables.
        std::unordered_map<std::string, std::string, shell_hash> m_mEnvVariables;
    };
} // namespace bs
