/**
 * @file command_env.cpp
 * @brief Implements commands for shell environment management.
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

#include "BashSpark/command/command_env.h"

#include "BashSpark/tools/shell_def.h"

namespace bs {

    shell_status command_getenv::run(
        const std::span<const std::string> &vArgs,
        shell_session &oSession
    ) const {
        if (vArgs.size() != 1) {
            this->msg_error_param_number(oSession.err(), vArgs.size());
            return shell_status::SHELL_CMD_ERROR_GETENV_PARAM_NUMBER;
        }

        // Check variable name
        const std::string &sVariable = vArgs[0];
        if (!is_var(sVariable)) {
            msg_error_variable_name(oSession.err(), sVariable);
            return shell_status::SHELL_CMD_ERROR_GETENV_VARIABLE_NAME_INVALID;
        }

        oSession.out() << oSession.get_env(sVariable);
        return shell_status::SHELL_SUCCESS;
    }

    void command_getenv::msg_error_param_number(std::ostream &oStdErr, const std::size_t nArgs) const {
        oStdErr << "getenv: takes 1 parameter, but received " << nArgs << "." << std::endl;
    }

    void command_getenv::msg_error_variable_name(std::ostream &oStdErr, const std::string &sVariableName) const {
        oStdErr << "getenv: \u201C" << sVariableName << "\u201D: not a variable name." << std::endl;
    }

    shell_status command_setenv::run(
        const std::span<const std::string> &vArgs,
        shell_session &oSession
    ) const {
        if (vArgs.size() != 2) {
            this->msg_error_param_number(oSession.err(), vArgs.size());
            return shell_status::SHELL_CMD_ERROR_SETENV_PARAM_NUMBER;
        }

        // Check variable name
        const std::string &sVariable = vArgs[0];
        if (!is_var(sVariable)) {
            msg_error_variable_name(oSession.err(), sVariable);
            return shell_status::SHELL_CMD_ERROR_GETENV_VARIABLE_NAME_INVALID;
        }

        oSession.env().set_env(sVariable, vArgs[1]);
        return shell_status::SHELL_SUCCESS;
    }

    void command_setenv::msg_error_param_number(std::ostream &oStdErr, const std::size_t nArgs) const {
        oStdErr << "setenv: takes 2 parameters, but received " << nArgs << "." << std::endl;
    }

    void command_setenv::msg_error_variable_name(std::ostream &oStdErr, const std::string &sVariableName) const {
        oStdErr << "setenv: \u201C" << sVariableName << "\u201D: not a variable name." << std::endl;
    }
}
