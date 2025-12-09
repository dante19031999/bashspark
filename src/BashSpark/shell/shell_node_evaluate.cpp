/**
 * @file shell_node_evaluate.cpp
 * @brief Implements class shell_node.
 *
 * This file contains the implementation of the `shell_node` class,
 * which provides helper tools for class `shell_parser`.
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

#include "BashSpark/shell/shell_node.h"
#include "BashSpark/tools/utf.h"
#include "BashSpark/shell.h"

#include "shell_tools.h"
#include "BashSpark/command/command_test.h"
#include "BashSpark/tools/shell_def.h"

namespace bs {
    shell_status shell_node_command::evaluate(shell_session &oSession) const {
        const auto pShell = oSession.get_shell();
        std::vector<std::string> vTokens;

        // Render command
        this->m_pCommand->expand(vTokens, oSession, true);
        ofakestream oStr;
        concat_vector(oStr, vTokens);
#ifdef BS_DEBUG
        std::cout << "cmd " << oStr.str() << std::endl;
#endif

        // Get command
        const auto pCommand = oSession.get_shell()->get_command(vTokens[0]);
        if (pCommand == nullptr) {
            pShell->msg_error_command_not_found(oSession, vTokens[0]);
            return shell_status::SHELL_ERROR_COMMAND_NOT_FOUND;
        }

        // Get args
        const std::span vArgs(vTokens.data() + 1, vTokens.size() - 1);
        auto nStatus = shell_status::SHELL_SUCCESS;

        // Run
        try {
            nStatus = pCommand->run(vArgs, oSession);
        } catch (shell_parser_exception &oException) {
            // Normalize syntax errors
            oSession.get_shell()->msg_error_syntax_error(oSession, oException);
        }

        // Establish result
        oSession.set_last_command_result(nStatus);
        return nStatus;
    }

    shell_status shell_node_null_command::evaluate(shell_session &oSession) const {
        // Establish result
        oSession.set_last_command_result(shell_status::SHELL_SUCCESS);
        return shell_status::SHELL_SUCCESS;
    }

    shell_status shell_node_command_block::evaluate(shell_session &oSession) const {
        for (const auto &pSubCommand: this->m_vSubCommands) {
            pSubCommand->evaluate(oSession);
        }
        return oSession.get_last_command_result();
    }

    shell_status shell_node_command_block_subshell::evaluate(shell_session &oSession) const {
        for (const auto &pSubCommand: this->m_vSubCommands) {
            auto pSubSession = oSession.make_subsession(
                oSession.in(),
                oSession.out(),
                oSession.err()
            );
            pSubCommand->evaluate(*pSubSession);
        }
        return oSession.get_last_command_result();
    }

    shell_status shell_node_background::evaluate(shell_session &oSession) const {
        return shell_status::SHELL_SUCCESS;
    }

    shell_status shell_node_and::evaluate(shell_session &oSession) const {
        const auto nLeft = this->get_left()->evaluate(oSession);
        if (nLeft == shell_status::SHELL_SUCCESS)
            return this->get_right()->evaluate(oSession);
        return nLeft;
    }

    shell_status shell_node_or::evaluate(shell_session &oSession) const {
        const auto nLeft = this->get_left()->evaluate(oSession);
        if (nLeft != shell_status::SHELL_SUCCESS)
            return this->get_right()->evaluate(oSession);
        return nLeft;
    }

    shell_status shell_node_pipe::evaluate(shell_session &oSession) const {
        std::ostringstream oStdOut;
        const auto pSessionLeft = oSession.make_pipe_left(oStdOut);
        this->get_left()->evaluate(*pSessionLeft);
        std::istringstream oStdIn(oStdOut.str());
        const auto pSessionRight = oSession.make_pipe_right(oStdIn);
        return this->get_right()->evaluate(*pSessionRight);
    }

    shell_status shell_node_test::evaluate(shell_session &oSession) const {
        std::vector<std::string> vTokens;

        // Render test
        this->m_pTest->expand(vTokens, oSession, true);
        ofakestream oStr;
        concat_vector(oStr, vTokens);

        // Get command
        std::unique_ptr<command_test> pCommand;
        auto pTest = oSession.get_shell()->get_command("test");
        if (pTest == nullptr) {
            pCommand = std::make_unique<command_test>();
            pTest = pCommand.get();
        }

        // Get args
        const std::span vArgs(vTokens.data(), vTokens.size());
        auto nStatus = shell_status::SHELL_SUCCESS;

        // Run
        try {
            nStatus = pTest->run(vArgs, oSession);
        } catch (shell_parser_exception &oException) {
            // Normalize syntax errors
            oSession.get_shell()->msg_error_syntax_error(oSession, oException);
        }

        // Establish result
        oSession.set_last_command_result(nStatus);
        return nStatus;
    }

    shell_status shell_node_if::evaluate(shell_session &oSession) const {
        if (
            const auto nCondition = this->m_pCondition->evaluate(oSession);
            nCondition == shell_status::SHELL_SUCCESS
        ) {
            return this->m_pCaseIf->evaluate(oSession);
        }
        if (this->m_pCaseElse != nullptr) {
            return this->m_pCaseElse->evaluate(oSession);
        }
        return oSession.get_last_command_result();
    }

    shell_status shell_node_continue::evaluate(shell_session &oSession) const {
        throw continue_signal{};
    }

    shell_status shell_node_break::evaluate(shell_session &oSession) const {
        throw break_signal{};
    }

    shell_status shell_node_for::evaluate(shell_session &oSession) const {
        std::vector<std::string> vSequence;
        this->m_pSequence->expand(vSequence, oSession, true);
        for (const auto &sItem: vSequence) {
            oSession.set_var(this->m_sVariable, sItem);
            try {
                this->m_pIterative->evaluate(oSession);
            } catch (shell_node_continue::continue_signal &) {
            } catch (shell_node_break::break_signal &) {
                break;
            }
        }
        return oSession.get_last_command_result();
    }

    shell_status shell_node_while::evaluate(shell_session &oSession) const {
        while (
            this->m_pCondition->evaluate(oSession) == shell_status::SHELL_SUCCESS
        ) {
            try {
                this->m_pIterative->evaluate(oSession);
            } catch (shell_node_continue::continue_signal &) {
            } catch (shell_node_break::break_signal &) {
                break;
            }
        }
        return oSession.get_last_command_result();
    }

    shell_status shell_node_until::evaluate(shell_session &oSession) const {
        while (
            this->m_pCondition->evaluate(oSession) != shell_status::SHELL_SUCCESS
        ) {
            try {
                this->m_pIterative->evaluate(oSession);
            } catch (shell_node_continue::continue_signal &) {
            } catch (shell_node_break::break_signal &) {
                break;
            }
        }
        return oSession.get_last_command_result();
    }

    shell_status shell_node_function::evaluate(shell_session &oSession) const {
        std::vector<std::string> vName;
        this->m_pName->expand(vName, oSession, true);
        if (vName.size() != 1 || !is_var(vName[0])) {
            ofakestream oStr;
            concat_vector(oStr, vName);
            oSession.get_shell()->msg_error_invalid_function_name(
                oSession, oStr.str()
            );
            return shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_FUNCTION_NAME;
        }
        oSession.set_func(vName[0], this->m_pBody.get());
        return shell_status::SHELL_SUCCESS;
    }
}
