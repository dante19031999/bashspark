/**
 * @file command_seq.cpp
 * @brief Implements command `bs::command_seq`.
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

#include "BashSpark/command/command_seq.h"

#include <limits>
#include <regex>

#include "BashSpark/tools/shell_def.h"

namespace bs {

    shell_status command_seq::run(const std::span<const std::string> &vArgs, shell_session &oSession) const {
        if (vArgs.size() != 2 && vArgs.size() != 3) {
            this->msg_error_param_number(oSession.err(), vArgs.size());
            return shell_status::SHELL_CMD_ERROR_SEQ_PARAM_NUMBER;
        }

        // Parse ints
        std::int64_t nArgs[3];
        for (std::size_t i = 0; i < vArgs.size(); ++i) {
            if (!is_number(vArgs[i])) {
                this->msg_error_int_format(oSession.err(), vArgs[i]);
                return shell_status::SHELL_CMD_ERROR_SEQ_INVALID_INT_FORMAT;
            }
            nArgs[i] = std::stoll(vArgs[i]);
        }

        // Iterate
        if (vArgs.size() == 2) {
            nArgs[2] = nArgs[1];
            nArgs[1] = nArgs[0] > nArgs[2] ? -1 : 1;
        } else {
            if (nArgs[0] > nArgs[2] && nArgs[1] >= 0) {
                this->msg_error_logics(oSession.err(), nArgs[0], nArgs[1], nArgs[2]);
                return shell_status::SHELL_CMD_ERROR_SEQ_ITERATION_LOGIC;
            }
            if (nArgs[0] < nArgs[2] && nArgs[1] <= 0) {
                this->msg_error_logics(oSession.err(), nArgs[0], nArgs[1], nArgs[2]);
                return shell_status::SHELL_CMD_ERROR_SEQ_ITERATION_LOGIC;
            }
        }

        oSession.out() << nArgs[0];
        if (nArgs[0] < nArgs[2]) {
            for (auto nIter = nArgs[0] + nArgs[1]; nIter <= nArgs[2]; nIter += nArgs[1]) {
                oSession.out().put(' ') << nIter;
            }
        } else if (nArgs[0] > nArgs[2]) {
            for (auto nIter = nArgs[0] + nArgs[1]; nIter >= nArgs[2]; nIter += nArgs[1]) {
                oSession.out().put(' ') << nIter;
            }
        }

        return shell_status::SHELL_SUCCESS;
    }

    void command_seq::msg_error_param_number(std::ostream &oStdErr, const std::size_t nArgs) const {
        oStdErr << "seq: takes 2-3 parameters, but received " << nArgs << "." << std::endl;
    }

    void command_seq::msg_error_int_format(std::ostream &oStdErr, const std::string &sInt) const {
        oStdErr << "seq: value \u201C" << sInt << "\u201D is no integer";
    }

    void command_seq::msg_error_int_bounds(std::ostream &oStdErr, const std::string &sInt) const {
        oStdErr << "seq: value \u201C" << sInt << "\u201D out of bounds "
                "[ " << std::numeric_limits<long long int>::min()
                << " : " << std::numeric_limits<long long int>::max()
                << " ]" << std::endl;
    }

    void command_seq::msg_error_logics(std::ostream &oStdErr, const std::int64_t nMin, const std::int64_t nStep,
                                       const std::int64_t nMax) const {
        oStdErr << "seq: can not iterate: "
                "[ " << nMin << " : " << nStep << " : " << nMax << " ]" << std::endl;
    }
}
