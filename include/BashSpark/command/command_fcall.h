/**
 * @file command_fcall.h
 * @brief Defines command `bs::command_fcall`.
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
     * @class command_fcall
     * @brief Calls a function
     *
     * Syntax: fcall funcion_name \<args\>
     *
     * Possible errors:
     * - `bs::shell_status::SHELL_CMD_ERROR_FCALL_FUNCTION_NOT_FOUND`: the function was not found.
     * - `bs::shell_status::SHELL_CMD_ERROR_FCALL_PARAM_NUMBER`: if there isn't at least one parameter.
     *
     */
    class command_fcall : public command {
    public:
        /**
         * @brief Constructs command
         */
        command_fcall()
            : command("fcall") {
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
         * @brief Print an error if the wrong number of arguments is provided.
         * @param oStdErr Stream to print error message.
         * @param nArgs Number of provided arguments.
         */
        virtual void msg_error_param_number(std::ostream &oStdErr, std::size_t nArgs) const;

        /**
         * @brief Displays the error message for function not found error
         *
         * Can be overwritten with custom behaviour.
         *
         * @param oStdErr Error output stream
         * @param sFunction Status code
         */
        virtual void msg_error_function_not_found(std::ostream &oStdErr, const std::string &sFunction) const;
    };
}
