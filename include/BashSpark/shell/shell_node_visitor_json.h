/**
 * @file shell_node_visitor_json.h
 * @brief Defines class `bs::shell_node_visitor_json`
 *
 * Defines class `bs::shell_node_visitor_json` to visit shell node structures.
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

#pragma once

#include <nlohmann/json.hpp>

#include "BashSpark/shell/shell_node_visitor.h"

namespace bs {
    /**
     * @class shell_node_visitor_json
     * @brief Visitor that converts shell nodes into JSON.
     *
     * This visitor traverses the shell AST (`shell_node` hierarchy) and produces a
     * `nlohmann::ordered_json` structure representing the node tree.
     * It is primarily intended as a debugging tool to visualize and understand
     * complex parser output.
     */
    class shell_node_visitor_json final : public shell_node_visitor<nlohmann::ordered_json> {
    public:
        /**
         * @brief Visit a generic shell node and return its JSON representation.
         *
         * Dispatches the node to the appropriate specialized `visit()` overload
         * inherited from `shell_node_visitor`, constructing a JSON object that
         * describes the node and its children.
         *
         * @param oSession The active shell session used during evaluation.
         * @param pRawNode Pointer to the node being visited.
         * @return A JSON object representing the visited node.
         */
        nlohmann::basic_json<nlohmann::ordered_map>
        visit_node(shell_session &oSession, const shell_node *pRawNode) override;

    private:
        /// @name Visit functions for each concrete shell node type.
        /// Implement these in derived classes.
        /// @{

        /**
         * @brief Visit a word node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_word *pNode) override;

        /**
         * @brief Visit a unicode node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_unicode *pNode) override;

        /**
         * @brief Visit a simple quoted string node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_str_simple *pNode) override;

        /**
         * @brief Visit a double-quoted string node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_str_double *pNode) override;

        /**
         * @brief Visit a backtick command string node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_str_back *pNode) override;

        /**
         * @brief Visit a null command node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_null_command *pNode) override;

        /**
         * @brief Visit a command node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_command *pNode) override;

        /**
         * @brief Visit a command expression node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_command_expression *pNode) override;

        /**
         * @brief Visit a command block node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_command_block *pNode) override;

        /**
         * @brief Visit a subshell command block node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_command_block_subshell *pNode) override;

        /**
         * @brief Visit an argument node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_arg *pNode) override;

        /**
         * @brief Visit a variable node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_variable *pNode) override;

        /**
         * @brief Visit a `$N` argument reference node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_dollar_arg *pNode) override;

        /**
         * @brief Visit a `$var` variable reference node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_dollar_variable *pNode) override;

        /**
         * @brief Visit a `${N}` argument reference node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_dollar_arg_dhop *pNode) override;

        /**
         * @brief Visit a `${var}` variable reference node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_dollar_variable_dhop *pNode) override;

        /**
         * @brief Visit a command substitution node (`$(...)`).
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_dollar_command *pNode) override;

        /**
         * @brief Visit a special variable node (`$?`, `$#`, etc.).
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_dollar_special *pNode) override;

        /**
         * @brief Visit a background operator node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_background *pNode) override;

        /**
         * @brief Visit a pipe operator node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_pipe *pNode) override;

        /**
         * @brief Visit a logical OR operator node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_or *pNode) override;

        /**
         * @brief Visit a logical AND operator node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_and *pNode) override;

        /**
         * @brief Visit a test command node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_test *pNode) override;

        /**
         * @brief Visit an if-statement node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_if *pNode) override;

        /**
         * @brief Visit a break statement node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_break *pNode) override;

        /**
         * @brief Visit a continue statement node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_continue *pNode) override;

        /**
         * @brief Visit a for-loop node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_for *pNode) override;

        /**
         * @brief Visit a while-loop node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_while *pNode) override;

        /**
         * @brief Visit an until-loop node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_until *pNode) override;

        /**
         * @brief Visit an until-loop node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The node as json.
         */
        visit_type visit(shell_session &oSession, const shell_node_function *pNode) override;

        /// @}
    };
}
