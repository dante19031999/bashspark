/**
 * @file shell.h
 * @brief Defines the `bs::shell` interface and built-in shell functionality.
 *
 * This header file provides the declaration of the `bs::shell` class, which serves
 * as the foundational interface for executing shell commands. It facilitates
 * command management, execution, and error handling while ensuring a structured
 * approach to building a secure and extensible command-line environment.
 * Users can leverage this interface to implement custom commands and
 * manage command behavior efficiently, reinforcing security and control
 * over command execution.
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

#include <memory>
#include <string_view>

#include "BashSpark/command.h"
#include "BashSpark/shell/shell_session.h"
#include "BashSpark/shell/shell_parser_exception.h"
#include "BashSpark/tools/shell_hash.h"

namespace bs {
    /**
     * @class shell
     * @brief A simplified shell environment inspired by Bash.
     *
     * This class provides functionalities for command execution, command management, and error handling.
     * Users can define and control commands while managing how errors are communicated.
     *
     * The command management methods allow the programmer to strictly control the allowed
     * behaviors, preventing security holes. This enhances the safety and reliability
     * of command execution, ensuring that only authorized commands are executed
     * and that errors are handled gracefully.
     *
     * It is optimized in the find direction, while the construction is more costly.
     * Uses a fixed size hash table bellow, which is expensive to initialize.
     * You may modify ref shell::DEFAULT_COMMAND_HASH_TABLE_SIZE to change that behavior.
     * In such case do not forget to rebuild the project.
     */
    class shell {
    public:
        /// Maximum depth the command interpreter can reach
        constexpr static std::size_t MAX_DEPTH = SHELL_MAX_DEPTH;

        /// Message name for bash syntax error
        constexpr static std::string_view BASH_ERROR_SYNTAX{"BASH_ERROR_SYNTAX"};

        /// Message name for bash command not found
        constexpr static std::string_view BASH_ERROR_COMMAND_NOT_FOUND{"BASH_ERROR_COMMAND_NOT_FOUND"};

        /// Default size for the command hash table
        constexpr static std::size_t DEFAULT_COMMAND_HASH_TABLE_SIZE = 1024;

    public:
        /**
         * @brief Construct bash object
         *
         * Included commands:
         * \ref bs::command_echo "Command echo"
         * \ref bs::command_eval "Command eval"
         * \ref bs::command_getenv "Command getenv"
         * \ref bs::command_getvar "Command getvar"
         * \ref bs::command_setenv "Command setenv"
         * \ref bs::command_setvar "Command setvar"
         * \ref bs::command_seq "Command seq"
         * \ref bs::command_test "Command test"
         */
        static std::unique_ptr<shell> make_default_shell();

    public:
        /**
         * @brief Construct bash object
         */
        shell()
            : m_mCommands(DEFAULT_COMMAND_HASH_TABLE_SIZE) {
        }

        /**
         * @brief Destruct bash object
         */
        virtual ~shell() = default;

    public:
        /**
         * @brief Runs a command or script
         * @param oCommand Command or script to run
         * @param oSession Shell session
         * @return Status code of last executed command
         */
        static shell_status run(
            std::istream &oCommand,
            shell_session &oSession
        );

        /**
         * @brief Runs a command or script
         * @param sCommand Command or script to run
         * @param oSession Shell session
         * @return Status code of last executed command
         */
        static shell_status run(
            const std::string &sCommand,
            shell_session &oSession
        );

        /**
         * @brief Runs a command or script
         * @param sCommand Command or script to run
         * @param oSession Shell session
         * @return Status code of last executed command
         */
        static shell_status run(
            const std::string_view &sCommand,
            shell_session &oSession
        );

    public:
        /**
          * @brief Retrieves a command pointer by its string.
          * @param sCommand The command string.
          * @return Pointer to the command, or nullptr if not found.
          */
        [[nodiscard]] const command *get_command(const std::string &sCommand) const noexcept;

        /**
          * @brief Sets a command using a unique pointer. Overwrites previous command.
          * @param pCommand Unique pointer to the command.
          */
        void set_command(std::unique_ptr<command> &&pCommand);

        /**
          * @brief Creates and sets a command of type CommandT.
          *
          * This template function constructs a command derived from the base class `command`
          * using the provided arguments and returns a unique pointer to it.
          *
          * @tparam CommandT The derived command type.
          * @param args Arguments to initialize the command.
          * @throws std::logic_error if CommandT is not derived from command.
          */
        template<typename CommandT, typename... Args>
        void set_command(Args &&... args) {
            static_assert(std::is_base_of_v<command, CommandT>,
                          "Template set_command only takes command derived classes.");
            std::unique_ptr<command> pCommand(new CommandT(std::forward<Args>(args)...));
            this->set_command(std::move(pCommand));
        }

        /**
          * @brief Removes a command and returns it.
          * @param sCommand The command string.
          * @return Unique pointer to the removed command, or nullptr if not found.
          */
        [[nodiscard]] std::unique_ptr<command> remove_command(const std::string &sCommand);

        /**
          * @brief Erases a command by its string.
          * @param sCommand The command string to erase.
          */
        void erase_command(const std::string &sCommand);

    public:
        /**
         * @brief Checks if execution stops on command not found.
         * @return true if it stops; false otherwise.
         */
        [[nodiscard]] bool get_stop_on_command_not_found() const noexcept;

        /**
         * @brief Sets whether to stop execution on command not found.
         * @param bStopOnCommandNotFound If true, execution stops; if false, it continues.
         */
        void set_stop_on_command_not_found(bool bStopOnCommandNotFound) noexcept;

    public:
        /**
         * @brief Displays the error message for “command not found”.
         *
         * Can be overwritten with custom behaviour.
         *
         * @param oSession Shell session
         * @param sCommand Command not found
         */
        virtual void msg_error_command_not_found(shell_session &oSession, const std::string &sCommand) const;

        /**
         * @brief Displays the error message for “function name invalid”.
         *
         * Can be overwritten with custom behaviour.
         *
         * @param oSession Shell session
         * @param sFunction Invalid function name
         */
        virtual void msg_error_invalid_function_name(shell_session &oSession, const std::string &sFunction) const;

        /**
         * @brief Displays the error message for “syntax errors”.
         *
         * Can be overwritten with custom behaviour.
         *
         * @param oSession Shell session
         * @param oException Syntax error
         */
        virtual void msg_error_syntax_error(
            const shell_session &oSession,
            const shell_parser_exception &oException
        ) const;

    private:
        /// Data structure to hold commands. Optimized for read time.
        std::unordered_map<std::string, std::unique_ptr<command>, shell_hash> m_mCommands;
        /// Mutex to allow only one command execution at once
        mutable std::mutex m_oExecutionMutex;
        /// Stop execution on command not found
        bool m_bStopOnCommandNotFound = true;
    };
}
