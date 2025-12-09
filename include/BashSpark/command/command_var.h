/**
 * @file command_var.h
 * @brief Defines commands for local variable management.
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
     * @class command_getvar
     * @brief Retrieves the value of an local variable.
     *
     * Syntax: getvar variable
     */
    class command_getvar : public command {
    public:
        /**
          * @brief Constructs command
          */
        command_getvar()
            : command("getvar") {
        }

        /**
         * @brief Prints the local variable value on stdout.
         * If the local variable does not exist, does nothing.
         *
         * If the parameter number does not match 1 then method `msg_error_param_number` is called.
         * If the variable names is invalid then method `msg_error_variable_name` is called.
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
         * @brief Print an error if the wrong number of arguments is provided.
         * @param oStdErr Stream to print error message.
         * @param nArgs Number of provided arguments.
         */
        virtual void msg_error_param_number(std::ostream &oStdErr, std::size_t nArgs) const;

        /**
         * @brief Print an error if variable name is invalid.
         * @param oStdErr Stream to print error message.
         * @param sVariableName Variable name provided.
         */
        virtual void msg_error_variable_name(std::ostream &oStdErr, const std::string &sVariableName) const;
    };


    /**
     * @class command_setvar
     * @brief Sets the value of a local variable.
     *
     * Syntax: setvar variable value
     */
    class command_setvar : public command {
    public:
        /**
         * @brief Constructs command
         */
        command_setvar()
            : command("setvar") {
        }

        /**
          * @brief Sets the local variable value.
          * If the local variable does not exist, creates it.
          *
          * If the parameter number does not match 2 then method `msg_error_param_number` is called.
          * If the variable names is invalid then method `msg_error_variable_name` is called.
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
         * @brief Print an error if the wrong number of arguments is provided.
         * @param oStdErr Stream to print error message.
         * @param nArgs Number of provided arguments.
         */
        virtual void msg_error_param_number(std::ostream &oStdErr, std::size_t nArgs) const;

        /**
         * @brief Print an error if variable name is invalid.
         * @param oStdErr Stream to print error message.
         * @param sVariableName Variable name provided.
         */
        virtual void msg_error_variable_name(std::ostream &oStdErr, const std::string &sVariableName) const;
    };
}
