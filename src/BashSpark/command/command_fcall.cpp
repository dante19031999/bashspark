/**
 * @file command_fcall.cpp
 * @brief Implements command `bs::command_fcall`.
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

#include "BashSpark/command/command_fcall.h"

#include "BashSpark/shell/shell_node.h"

namespace bs {
    shell_status command_fcall::run(const std::span<const std::string> &vArgs, shell_session &oSession) const {
        std::cout << "FCALL" << std::endl;
        // Check args
        if (vArgs.empty()) {
            this->msg_error_param_number(oSession.err(), vArgs.size());
            return shell_status::SHELL_CMD_ERROR_FCALL_PARAM_NUMBER;
        }

        // Check function name
        const auto pFunc = oSession.get_func(vArgs[0]);
        if (pFunc == nullptr) {
            this->msg_error_function_not_found(oSession.err(), vArgs[0]);
            return shell_status::SHELL_CMD_ERROR_FCALL_FUNCTION_NOT_FOUND;
        }

        // Run
        std::vector vFuncArgs(vArgs.begin(), vArgs.end());
        shell_arg oArgs(std::move(vFuncArgs));
        const auto pSession = oSession.make_function_call(std::move(oArgs));
        std::cout << "arg 1: " << pSession->get_arg(1) << std::endl;
        return pFunc->evaluate(*pSession);
    }

    void command_fcall::msg_error_param_number(std::ostream &oStdErr, const std::size_t nArgs) const {
        oStdErr << "fcall: takes >=1 parameters, but received " << nArgs << "." << std::endl;
    }

    void command_fcall::msg_error_function_not_found(std::ostream &oStdErr, const std::string &sFunction) const {
        oStdErr << "fcall: " << sFunction << ": function not found." << std::endl;
    }
}
