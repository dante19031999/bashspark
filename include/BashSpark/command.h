/**
 * @file command.h
 * @brief Defines the ::bs::command interface and built-in shell commands.
 *
 * This file provides the base class `command` for shell commands and several
 * concrete commands such as `echo`, `eval`, `getenv`, `getvar`, `setenv`,
 * `setvar`, and `seq`. Each command can be executed within a shell session
 * and provides virtual error-handling methods to allow custom behavior,
 * including internationalization.
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

#include <span>

#include "BashSpark/shell/shell_session.h"

namespace bs {
    class shell;

    /**
     * @class command
     * @brief Abstract base class for all shell commands.
     *
     * Encapsulates a shell command's identity and execution interface. Every
     * command must implement `run()` to perform its action. Commands may be
     * extended to provide custom error reporting.
     */
    class command {
        friend class shell;

    public:
        /**
          * @brief Construct a command with a given name.
          * @param sName The command's name.
          */
        explicit command(std::string sName)
            : m_sName(std::move(sName)) {
        }

    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~command() = default;

    public:
        /**
         * @brief Execute the command.
         *
         * @param vArgs Arguments for the command.
         * @param oSession The shell session context.
         * @return Status of command execution.
         */
        [[nodiscard]] virtual shell_status run(
            const std::span<const std::string> &vArgs,
            shell_session &oSession
        ) const = 0;

    public:
        /**
         * @brief Get the command name (copy).
         * @return Command name.
         */
        [[nodiscard]] std::string get_name() const {
            return this->m_sName;
        }

        /**
         * @brief Get the command name (reference).
         * @return Reference to command name string.
         */
        [[nodiscard]] const std::string &get_name_ref() const noexcept {
            return this->m_sName;
        }

    private:
        /// Command name
        std::string m_sName;
    };

    /**
      * @class command_echo
      * @brief Built-in command that prints all arguments to stdout.
      *
      * Syntax: echo [argument]?+ (with new line)<br>
      * Syntax: echo -n [argument]?+ (without new line)
      *
      */
    class command_echo : public command {
    public:
        /**
         * @brief Constructs the command
         */
        command_echo()
            : command("echo") {
        }

    public:
        /**
         * @brief Prints all arguments to stdout.
         *
         * Arguments are printed separated by a single space. Supports any number
         * of arguments. No formatting or escape sequences are processed.
         *
         * Option -n as first parameter disables  '\n' at the end of output.
         *
         * @param vArgs Arguments for the command.
         * @param oSession The shell session context.
         * @return Status of command execution.
         */
        [[nodiscard]] shell_status run(
            const std::span<const std::string> &vArgs,
            shell_session &oSession
        ) const override;
    };

    /**
     * @class command_eval
     * @brief Built-in command that evaluates its arguments as a shell command.
     *
     * Executes the provided string(s) as if typed in the shell. This command
     * allows dynamic command execution at runtime.
     */
    class command_eval : public command {
    public:
        /**
         * @brief Constructs the command
         */
        explicit command_eval()
            : command("eval") {
        }

    public:
        /**
         * @brief Execute the provided arguments as if typed in shell.
         * The corresponding status code is returned.
         *
         * @param vArgs Arguments for the command.
         * @param oSession The shell session context.
         * @return Status of command execution.
         */
        [[nodiscard]] shell_status run(
            const std::span<const std::string> &vArgs,
            shell_session &oSession
        ) const override;

    protected:
        /**
         * @brief Print an error if the depth limit of the shell is surpassed.
         * @param oStdErr Stream to print error message.
         */
        virtual void msg_error_max_depth_reached(std::ostream &oStdErr) const;
    };
}
