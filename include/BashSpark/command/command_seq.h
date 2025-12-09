/**
 * @file command_seq.h
 * @brief Defines command `bs::command_seq`.
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
     * @class command_seq
     * @brief Prints a sequence on stdout.
     * Supports negative numbers and decreasing sequences.
     *
     * Syntax: seq begin end <br>
     * Syntax: seq begin step end
     *
     */
    class command_seq : public command {
    public:
        /**
         * @brief Constructs command
         */
        command_seq()
            : command("seq") {
        }

        /**
         * @brief Prints a sequence on stdout.
         *
         * Integers are defined by regex: R"(^[+-]?\d+$)"
         *
         * If the parameter count does not match 2 or 3 then `msg_error_param_number` method is called.
         * If any of the parameters is not a number then `msg_error_int_format` method is called.
         * If any of the parameters is not within the bounds of c2 64 bit binary then `msg_error_int_bounds` method is called.
         * If the parameters are incoherent, meaning the sequence can not be completed, then `msg_error_logics` method is called.
         *
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
         * @brief Print an error if a parameter is non integer.
         * @param oStdErr Stream to print error message.
         * @param sInt Non integer parameter.
         */
        virtual void msg_error_int_format(std::ostream &oStdErr, const std::string &sInt) const;

        /**
         * @brief Print an error if an integer values is outside the bounds of c2 64 bit binary.
         * @param oStdErr Stream to print error message.
         * @param sInt Non integer parameter.
         */
        virtual void msg_error_int_bounds(std::ostream &oStdErr, const std::string &sInt) const;

        /**
         * @brief Print an error if the parameters are incoherent, meaning the sequence can not be completed.
         * @param oStdErr Stream to print error message.
         * @param nMin Starting value of the sequence
         * @param nStep Stepping value of the sequence
         * @param nMax Final value of the sequence
         */
        virtual void msg_error_logics(std::ostream &oStdErr, std::int64_t nMin, std::int64_t nStep,
                                      std::int64_t nMax) const;
    };
}
