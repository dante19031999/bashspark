/**
 * @file command.cpp
 * @brief Implements class ::bs::command.
 *
 * Provides an interface define shell commands.
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

#include "BashSpark/command.h"

#include <limits>
#include <ostream>
#include <regex>
#include <sstream>

#include <BashSpark/shell/shell_parser.h>

#include "shell/shell_tools.h"
#include "BashSpark/shell/shell_session.h"
#include "BashSpark/tools/fakestream.h"
#include "BashSpark/tools/hash.h"

namespace bs {
    shell_status command_echo::run(
        const std::span<const std::string> &vArgs,
        shell_session &oSession
    ) const {
        // Se
        bool bUseEndl = true;
        std::size_t nBegin = 0;

        // Process options
        if (!vArgs.empty()) {
            if (vArgs[0] == "-n") {
                bUseEndl = false;
                nBegin = 1;
            }
        }

        // Temporal ostream
        // This prevents to some point mix-up on multithread environments
        ofakestream oOstream;

        // Print args from the beginning
        if (vArgs.size() > nBegin)
            oOstream << vArgs[nBegin];
        for (std::size_t i = nBegin + 1; i < vArgs.size(); i++) {
            oOstream.put(' ');
            oOstream << vArgs[i];
        }

        // Print endl
        if (bUseEndl)
            oOstream.put('\n');

        // Print final
        oSession.out() << oOstream.view();
        return shell_status::SHELL_SUCCESS;
    }

    shell_status command_eval::run(
        const std::span<const std::string> &vArgs,
        shell_session &oSession
    ) const {
        // Build string
        ofakestream oCommand;
        if (!vArgs.empty()) {
            oCommand << vArgs[0];
            for (std::size_t i = 1; i < vArgs.size(); i++) {
                oCommand << vArgs[i];
            }
        }

        // Increase depth
        if (!oSession.increase_shell_depth()) {
            this->msg_error_max_depth_reached(oSession.err());
            return shell_status::SHELL_ERROR_MAX_DEPTH_REACHED;
        }

        // Fetch string and run
        const auto nStatus = shell::run(oCommand.view(), oSession);

        // Clean and return
        oSession.decrease_shell_depth();
        return nStatus;
    }

    void command_eval::msg_error_max_depth_reached(std::ostream &oStdErr) const {
        oStdErr << "Maximum shell depth reached." << std::endl;
    }
}
