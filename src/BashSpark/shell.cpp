/**
 * @file shell.cpp
 * @brief Implements the `bs::shell` interface and built-in shell functionality.
 *
 * This source file provides the implementation of the `bs::shell` class, which serves
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

#include "BashSpark/shell.h"

#include "BashSpark/command/command_env.h"
#include "BashSpark/command/command_fcall.h"
#include "BashSpark/command/command_math.h"
#include "BashSpark/command/command_seq.h"
#include "BashSpark/command/command_test.h"
#include "BashSpark/command/command_var.h"
#include "BashSpark/shell/shell_node_visitor_json.h"
#include "BashSpark/shell/shell_parser.h"
#include "BashSpark/tools/fakestream.h"
#include "BashSpark/tools/hash.h"
#include "BashSpark/tools/nullstream.h"

namespace bs {
    std::unique_ptr<shell> shell::make_default_shell() {
        auto pShell = std::make_unique<shell>();
        pShell->set_command<command_echo>();
        pShell->set_command<command_eval>();
        pShell->set_command<command_getenv>();
        pShell->set_command<command_getvar>();
        pShell->set_command<command_setenv>();
        pShell->set_command<command_setvar>();
        pShell->set_command<command_seq>();
        pShell->set_command<command_test>();
        pShell->set_command<command_math>();
        pShell->set_command<command_fcall>();
        return pShell;
    }

    const command *shell::get_command(const std::string &sCommand) const noexcept {
        const auto pIter = this->m_mCommands.find(sCommand);
        if (pIter == this->m_mCommands.end())
            return nullptr;
        return pIter->second.get();
    }

    void shell::set_command(std::unique_ptr<command> &&pCommand) {
        this->m_mCommands[pCommand->get_name_ref()] = std::move(pCommand);
    }

    std::unique_ptr<command> shell::remove_command(const std::string &sCommand) {
        // Search external map
        const auto pIter = this->m_mCommands.find(sCommand);
        if (pIter == this->m_mCommands.end())
            return nullptr;
        auto pCommand = std::move(pIter->second);
        this->m_mCommands.erase(pIter);
        return pCommand;
    }

    void shell::erase_command(const std::string &sCommand) {
        if (
            const auto pIter = this->m_mCommands.find(sCommand);
            pIter != this->m_mCommands.end()
        ) {
            this->m_mCommands.erase(pIter);
        }
    }

    bool shell::get_stop_on_command_not_found() const noexcept {
        return this->m_bStopOnCommandNotFound;
    }

    void shell::set_stop_on_command_not_found(const bool bStopOnCommandNotFound) noexcept {
        this->m_bStopOnCommandNotFound = bStopOnCommandNotFound;
    }

    void shell::msg_error_command_not_found(shell_session &oSession, const std::string &sCommand) const {
        oSession.err() << "shell: \u201C" << sCommand << "\u201D: not found." << std::endl;
    }

    void shell::msg_error_invalid_function_name(shell_session &oSession, const std::string &sFunction) const {
        oSession.err() << "shell: \u201C" << sFunction << "\u201D: invalid function name." << std::endl;
    }

    void shell::msg_error_syntax_error(const shell_session &oSession, const shell_parser_exception &oException) const {
        oSession.err() << oException.what();
    }

    namespace {
        ALWAYS_INLINE shell_status eval(
            shell_session &oSession,
            ifakestream &oIstream
        ) {
            const auto pMainNode = shell_parser::parse(oIstream);
            //shell_node_visitor_json oVisitor;
            //auto sJson = oVisitor.visit_node(oSession, pMainNode.get());
            //std::ofstream oFile("/home/$USER/Documents/BashSpark/node.json");
            //oFile << oVisitor.visit_node(oSession, pMainNode.get()).dump(4) << std::endl;
            return pMainNode->evaluate(oSession);
            try {
                const auto pMainNode = shell_parser::parse(oIstream);
                return pMainNode->evaluate(oSession);
            } catch (const shell_parser_exception &oException) {
                oSession.get_shell()->msg_error_syntax_error(
                    oSession, oException
                );
                return oException.get_status();
            }
        }
    }

    shell_status shell::run(const std::string &sCommand, shell_session &oSession) {
        // Open fake istream
        ifakestream oIstream(sCommand.data(), sCommand.length());
        // Run command
        return eval(oSession, oIstream);
    }

    shell_status shell::run(const std::string_view &sCommand, shell_session &oSession) {
        // Open fake istream
        ifakestream oIstream(sCommand.data(), sCommand.length());
        // Run command
        return eval(oSession, oIstream);
    }

    shell_status shell::run(std::istream &oCommand, shell_session &oSession) {
        // Parse istream
        const std::string sCommand(std::istreambuf_iterator<char>(oCommand), {});
        // Open fake istream
        ifakestream oIstream(sCommand.data(), sCommand.length());
        // Run command
        return eval(oSession, oIstream);
    }
}
