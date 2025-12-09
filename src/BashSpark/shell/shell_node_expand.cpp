/**
 * @file shell_node_expand.cpp
 * @brief Implements class shell_node.
 *
 * This file implementation the definition of the `shell_node` class,
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
#include "BashSpark/tools/shell_def.h"

namespace bs {
    void shell_node_command_expression::expand(
        std::vector<std::string> &vTokens,
        shell_session &oSession,
        const bool) const {
        ofakestream oWord;

        for (const auto &pChild: this->m_vChildren) {
            if (pChild == nullptr) {
                if (!oWord.empty()) {
                    vTokens.push_back(oWord.str_reset());
                }
            } else {
                std::vector<std::string> vSubTokens;
                pChild->expand(vSubTokens, oSession, true);
                if (!vSubTokens.empty()) {
                    // Process start, first word is sticken to pre sub expandable
                    oWord << vSubTokens.front();
                    vTokens.push_back(oWord.str_reset());
                    // Process middle, spaces are real breaks
                    if (vSubTokens.size() > 2) {
                        for (std::size_t i = 1; i < vSubTokens.size() - 1; ++i) {
                            vTokens.push_back(vSubTokens[i]);
                        }
                    }
                    // Precess end, final word is sticken to pos sub expandable
                    if (vSubTokens.size() > 1) {
                        oWord << vSubTokens.back();
                    }
                }
            }
        }

        if (!oWord.empty()) {
            vTokens.push_back(oWord.str());
        }

        ofakestream oStr;
        concat_vector(oStr, vTokens);
    }

    void shell_node_word::expand(
        std::vector<std::string> &vTokens,
        shell_session &oSession,
        bool) const {
        vTokens.push_back(this->m_sText);
    }

    void shell_node_unicode::expand(
        std::vector<std::string> &vTokens,
        shell_session &oSession,
        bool) const {
        ofakestream oOstream;
        write_char32_t(oOstream, this->m_cCharacter);
        vTokens.push_back(oOstream.str());
    }

    void shell_node_str::expand(
        std::vector<std::string> &vTokens,
        shell_session &oSession,
        bool) const {
        ofakestream oOstream;
        for (const auto &pChild: m_vChildren) {
            std::vector<std::string> vSubTokens;
            pChild->expand(vSubTokens, oSession, false);
            for (const auto &oSubExpansion: vSubTokens) {
                oOstream << oSubExpansion;
            }
        }
        vTokens.push_back(oOstream.str());
    }

    void shell_node_str_back::expand(
        std::vector<std::string> &vTokens,
        shell_session &oSession,
        const bool bSplit
    ) const {
        std::ostringstream oStdOut;
        const auto pSubSession = oSession.make_subsession(
            oSession.in(),
            oStdOut,
            oSession.out()
        );
        this->m_pCommand->evaluate(*pSubSession);
        if (bSplit)
            split_string(vTokens, oStdOut.view());
        else
            vTokens.push_back(oStdOut.str());
    }

    void shell_node_session_extractor::expand(
        std::vector<std::string> &vTokens,
        shell_session &oSession,
        const bool bSplit
    ) const {
        if (bSplit)
            split_string(vTokens, this->get_value(oSession));
        else
            vTokens.push_back(this->get_value(oSession));
    }


    std::string shell_node_arg::get_value(
        const shell_session &oSession
    ) const {
        return oSession.get_arg(m_nArg);
    }

    std::string shell_node_dollar_arg::get_value(
        const shell_session &oSession
    ) const {
        return oSession.get_arg(m_nArg);
    }

    std::string shell_node_variable::get_value(
        const shell_session &oSession
    ) const {
        if (oSession.has_var(m_sVariable))
            return oSession.get_var(m_sVariable);
        if (oSession.has_env(m_sVariable))
            return oSession.get_env(m_sVariable);
        return "";
    }

    std::string shell_node_dollar_variable::get_value(
        const shell_session &oSession
    ) const {
        if (oSession.has_var(m_sVariable))
            return oSession.get_var(m_sVariable);
        if (oSession.has_env(m_sVariable))
            return oSession.get_env(m_sVariable);
        return "";
    }

    std::string shell_node_dollar_arg_dhop::get_value(
        const shell_session &oSession
    ) const {
        const auto sArg = oSession.get_arg(m_nArg);
        if (is_number(sArg))
            return oSession.get_arg(std::stoull(sArg));
        if (oSession.has_var(sArg))
            return oSession.get_var(sArg);
        if (oSession.has_env(sArg))
            return oSession.get_env(sArg);
        return "";
    }

    std::string shell_node_dollar_variable_dhop::get_value(
        const shell_session &oSession
    ) const {
        std::string sVar;
        if (oSession.has_var(m_sVariable))
            sVar = oSession.get_var(m_sVariable);
        else if (oSession.has_env(m_sVariable))
            sVar = oSession.get_env(m_sVariable);
        else return "";
        if (is_number(sVar))
            return oSession.get_arg(std::stoull(sVar));
        if (oSession.has_var(sVar))
            return oSession.get_var(sVar);
        if (oSession.has_env(sVar))
            return oSession.get_env(sVar);
        return "";
    }

    void shell_node_dollar_command::expand(
        std::vector<std::string> &vTokens,
        shell_session &oSession,
        const bool bSplit
    ) const {
        std::ostringstream oStdOut;
        const auto pSubSession = oSession.make_subsession(
            oSession.in(),
            oStdOut,
            oSession.err()
        );
        this->m_pCommand->evaluate(*pSubSession);
        if (bSplit)
            split_string(vTokens, oStdOut.view());
        else
            vTokens.push_back(oStdOut.str());

        ofakestream oStr;
        concat_vector(oStr, vTokens);
    }

    std::string shell_node_dollar_special::get_value(
        const shell_session &oSession
    ) const {
        switch (this->m_cItem) {
            case '0':
                return "emptyset";
            case '$':
                return std::to_string(get_pid());
            case '?': {
                const auto nStatus = static_cast<int>(oSession.get_last_command_result());
                return std::to_string(nStatus);
            }
            case '#':
                // Args start one 1
                return std::to_string(oSession.get_arg_size()-1);
            case '@': {
                ofakestream oArgs;
                const std::vector<std::string> &vArgs = oSession.get_args();
                if (vArgs.size() > 1) {
                    oArgs << vArgs[1];
                    for (auto i = 2; i < vArgs.size(); ++i) {
                        oArgs.put(' ');
                        oArgs << vArgs[i];
                    }
                }
                return oArgs.str();
            }
            default:
                return "$";
        }
    }
}
