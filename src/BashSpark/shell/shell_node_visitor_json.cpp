/**
 * @file shell_node_visitor_json.cpp
 * @brief Implements class `bs::shell_node_visitor_json`
 *
 * Implements class `bs::shell_node_visitor_json` to visit shell node structures.
 * It is very useful to debug as it allows to understand the node structure better.
 * This particular visitor converts the shell node structure into json, allowing visual
 * understanding of the node structure.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "BashSpark/shell/shell_node_visitor_json.h"

#include "BashSpark/shell/shell_session.h"
#include "BashSpark/tools/nullstream.h"

namespace bs {
    /// Typedef of `shell_node_visitor_json::visit_type` (sugar syntax)
    using visit_type = shell_node_visitor_json::visit_type;

    // Visit function for the base case
    nlohmann::basic_json<nlohmann::ordered_map> shell_node_visitor_json::visit_node(
        shell_session &oSession, const shell_node *pRawNode) {
        auto oJson = shell_node_visitor::visit_node(oSession, pRawNode);

        // Expandable
        if (const auto pNode = dynamic_cast<const shell_node_expandable *>(pRawNode)) {
            inullstream oStdIn;
            onullstream oStdOut;
            onullstream oStdErr;
            auto pSubsession = oSession.make_subsession(oStdIn, oStdOut, oStdErr);

            std::ostringstream oStream;
            std::vector<std::string> vTokens;
            pNode->expand(vTokens, *pSubsession, true);
            for (const auto &sToken: vTokens) {
                oStream.put('[') << sToken << ']';
            }
            oJson["expansion"] = oStream.str();
        }

        // Evaluable
        if (const auto pNode = dynamic_cast<const shell_node_evaluable *>(pRawNode)) {
            inullstream oStdIn;
            onullstream oStdOut;
            onullstream oStdErr;
            auto pSubsession = oSession.make_subsession(oStdIn, oStdOut, oStdErr);

            auto nStatus = shell_status::SHELL_SUCCESS;
            try {
                nStatus = pNode->evaluate(*pSubsession);
            } catch (shell_node_continue::continue_signal &) {
            } catch (shell_node_break::break_signal &) {
            }
            oJson["evaluation"] = nStatus;
        }

        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_word *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "word";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["text"] = pNode->get_text();
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_unicode *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "unicode";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["char"] = pNode->get_character();
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_str_simple *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "str simple";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["children"] = nlohmann::ordered_json::array();
        auto &oChildren = oJson["children"];
        for (const auto &pChild: pNode->get_children()) {
            oChildren.push_back(this->visit_node(oSession, pChild.get()));
        }
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_str_double *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "str double";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["children"] = nlohmann::ordered_json::array();
        auto &oChildren = oJson["children"];
        for (const auto &pChild: pNode->get_children()) {
            oChildren.push_back(this->visit_node(oSession, pChild.get()));
        }
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_str_back *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "str back";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["command"] = this->visit_node(oSession, pNode->get_command());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_null_command *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "null cmd";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        return oJson;
    }

    // Visit function for command nodes
    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_command *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "cmd";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["expression"] = this->visit_node(oSession, pNode->get_command());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_command_expression *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "cmd exp";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["children"] = nlohmann::ordered_json::array();
        auto &oChildren = oJson["children"];
        for (const auto &pChild: pNode->get_children()) {
            if (pChild != nullptr)
                oChildren.push_back(this->visit_node(oSession, pChild.get()));
            else
                oChildren.push_back(nullptr);
        }
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_command_block *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "cmd block";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["children"] = nlohmann::ordered_json::array();
        auto &oChildren = oJson["children"];
        for (const auto &pChild: pNode->get_children()) {
            oChildren.push_back(this->visit_node(oSession, pChild.get()));
        }
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_command_block_subshell *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "cmd block sh";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["children"] = nlohmann::ordered_json::array();
        auto &oChildren = oJson["children"];
        for (const auto &pChild: pNode->get_children()) {
            oChildren.push_back(this->visit_node(oSession, pChild.get()));
        }
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_arg *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "arg";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["arg"] = pNode->get_arg();
        oJson["value"] = pNode->get_value(oSession);
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_variable *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "var";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["variable"] = pNode->get_variable();
        oJson["value"] = pNode->get_value(oSession);
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_dollar_arg *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "$arg";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["arg"] = pNode->get_arg();
        oJson["value"] = pNode->get_value(oSession);
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_dollar_variable *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "$var";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["variable"] = pNode->get_variable();
        oJson["value"] = pNode->get_value(oSession);
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_dollar_arg_dhop *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "$arg2";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["arg"] = pNode->get_arg();
        oJson["value"] = pNode->get_value(oSession);
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_dollar_variable_dhop *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "$var2";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["variable"] = pNode->get_variable();
        oJson["value"] = pNode->get_value(oSession);
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_dollar_command *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "$cmd";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["command"] = this->visit_node(oSession, pNode->get_command());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_dollar_special *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "$@";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["item"] = std::string{1, pNode->get_item()};
        oJson["value"] = pNode->get_value(oSession);
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_background *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "&";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["cmd"] = this->visit_node(oSession, pNode->get_command());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_pipe *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "|";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["left"] = this->visit_node(oSession, pNode->get_left());
        oJson["right"] = this->visit_node(oSession, pNode->get_right());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_or *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "||";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["left"] = this->visit_node(oSession, pNode->get_left());
        oJson["right"] = this->visit_node(oSession, pNode->get_right());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_and *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "&&";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["left"] = this->visit_node(oSession, pNode->get_left());
        oJson["right"] = this->visit_node(oSession, pNode->get_right());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_test *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "[]";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["test"] = this->visit_node(oSession, pNode->get_test());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_if *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "if";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["condition"] = this->visit_node(oSession, pNode->get_condition());
        oJson["case-if"] = this->visit_node(oSession, pNode->get_case_if());
        const auto pElse = pNode->get_case_else();
        oJson["case-else"] = pElse != nullptr ? this->visit_node(oSession, pElse) : nullptr;
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_break *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "break";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_continue *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "continue";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_for *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "for";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["variable"] = pNode->get_variable();
        oJson["sequence"] = this->visit_node(oSession, pNode->get_sequence());
        oJson["iterative"] = this->visit_node(oSession, pNode->get_iterative());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_while *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "while";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["condition"] = this->visit_node(oSession, pNode->get_condition());
        oJson["iterative"] = this->visit_node(oSession, pNode->get_iterative());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_until *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "until";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["condition"] = this->visit_node(oSession, pNode->get_condition());
        oJson["iterative"] = this->visit_node(oSession, pNode->get_iterative());
        return oJson;
    }

    visit_type shell_node_visitor_json::visit(shell_session &oSession, const shell_node_function *pNode) {
        nlohmann::ordered_json oJson;
        oJson["type"] = "function";
        oJson["evaluation"] = nullptr;
        oJson["expansion"] = nullptr;
        oJson["name"] = this->visit_node(oSession, pNode->get_name());
        oJson["body"] = this->visit_node(oSession, pNode->get_body());
        return oJson;
    }
}
