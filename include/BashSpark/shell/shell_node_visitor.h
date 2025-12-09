/**
 * @file shell_node_visitor.h
 * @brief Defines template `bs::shell_node_visitor`
 *
 * Provides a template to visit shell node structures.
 * It is very useful to debug as it allows to understand the node structure better.
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

#include "BashSpark/shell/shell_node.h"

namespace bs {
    /**
     * @brief Generic visitor for shell AST nodes.
     *
     * This class implements a type–safe visitor pattern over all `shell_node`
     * subclasses. Users must implement all `visit()` overloads corresponding
     * to the node types they wish to handle.
     *
     * @tparam visit_t The return type of all visit functions.
     * Must be default-constructible.
     */
    template<typename visit_t>
    class shell_node_visitor {
        static_assert(std::is_default_constructible_v<visit_t>,
                      "visit_t must be default-constructible");

    public:
        /// Type of data resulting of visit
        using visit_type = visit_t;

        virtual ~shell_node_visitor() = default;

        /**
         * @brief Dispatch a node to the correct visit function.
         *
         * Performs a type switch on the node's `shell_node_type` and forwards
         * it to the corresponding `visit()` overload. If `visit_t` is `void`,
         * no value is returned.
         *
         * @param oSession Execution session.
         * @param pRawNode Pointer to the node being visited.
         * @return The result of the dispatched visit, or a default-constructed
         *         `visit_t` if the node type is unhandled.
         */
        virtual visit_t visit_node(shell_session &oSession,
                                   const shell_node *pRawNode);

    protected:
        /// @name Visit functions for each concrete shell node type.
        /// Implement these in derived classes.
        /// @{

        /**
         * @brief Visit a word node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_word *pNode) = 0;

        /**
         * @brief Visit a Unicode node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_unicode *pNode) = 0;

        /**
         * @brief Visit a simple quoted string node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_str_simple *pNode) = 0;

        /**
         * @brief Visit a double-quoted string node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_str_double *pNode) = 0;

        /**
         * @brief Visit a backtick command string node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_str_back *pNode) = 0;

        /**
         * @brief Visit a null command node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_null_command *pNode) = 0;

        /**
         * @brief Visit a command node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_command *pNode) = 0;

        /**
         * @brief Visit a command expression node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_command_expression *pNode) = 0;

        /**
         * @brief Visit a command block node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_command_block *pNode) = 0;

        /**
         * @brief Visit a subshell command block node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_command_block_subshell *pNode) = 0;

        /**
         * @brief Visit an argument node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_arg *pNode) = 0;

        /**
         * @brief Visit a variable node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_variable *pNode) = 0;

        /**
         * @brief Visit a `$N` argument reference node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_dollar_arg *pNode) = 0;

        /**
         * @brief Visit a `$var` variable reference node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_dollar_variable *pNode) = 0;

        /**
         * @brief Visit a `${N}` argument reference node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_dollar_arg_dhop *pNode) = 0;

        /**
         * @brief Visit a `${var}` variable reference node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_dollar_variable_dhop *pNode) = 0;

        /**
         * @brief Visit a command substitution node (`$(...)`).
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_dollar_command *pNode) = 0;

        /**
         * @brief Visit a special variable node (`$?`, `$#`, etc.).
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_dollar_special *pNode) = 0;

        /**
         * @brief Visit a background operator node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_background *pNode) = 0;

        /**
         * @brief Visit a pipe operator node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_pipe *pNode) = 0;

        /**
         * @brief Visit a logical OR operator node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_or *pNode) = 0;

        /**
         * @brief Visit a logical AND operator node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_and *pNode) = 0;

        /**
         * @brief Visit a test command node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_test *pNode) = 0;

        /**
         * @brief Visit an if-statement node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_if *pNode) = 0;

        /**
         * @brief Visit a break statement node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_break *pNode) = 0;

        /**
         * @brief Visit a continue statement node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_continue *pNode) = 0;

        /**
         * @brief Visit a for-loop node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_for *pNode) = 0;

        /**
         * @brief Visit a while-loop node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_while *pNode) = 0;

        /**
         * @brief Visit an until-loop node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_until *pNode) = 0;

        /**
         * @brief Visit a function node.
         * @param oSession The current shell session.
         * @param pNode The node to visit.
         * @return The visitor result.
         */
        virtual visit_t visit(shell_session &oSession, const shell_node_function *pNode) = 0;

        /// @}
    };

    template<typename visit_t>
    shell_node_visitor<visit_t>::visit_type shell_node_visitor<visit_t>::visit_node(
        shell_session &oSession, const shell_node *pRawNode) {
        switch (pRawNode->get_type()) {
            case shell_node_type::SNT_COMMAND_EXPRESSION: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_command_expression *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_STR_SIMPLE: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_str_simple *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_STR_DOUBLE: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_str_double *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_STR_BACK: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_str_back *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_WORD: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_word *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_UNICODE: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_unicode *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_ARG: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_arg *>(pRawNode);

                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_VARIABLE: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_variable *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_DOLLAR_SPECIAL: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_dollar_special *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_DOLLAR_VARIABLE: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_dollar_variable *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_DOLLAR_VARIABLE_DHOP: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_dollar_variable_dhop *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_DOLLAR_ARG: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_dollar_arg *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_DOLLAR_ARG_DHOP: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_dollar_arg_dhop *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_DOLLAR_COMMAND: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_dollar_command *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_BACKGROUND: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_background *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_AND: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_and *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_PIPE: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_pipe *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_OR: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_or *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_IF: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_if *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_TEST: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_test *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_FOR: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_for *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_WHILE: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_while *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_UNTIL: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_until *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_BREAK: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_break *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_CONTINUE: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_continue *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_NULL_COMMAND: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_null_command *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_COMMAND: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_command *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_COMMAND_BLOCK: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_command_block *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_COMMAND_BLOCK_SUBSHELL: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_command_block_subshell *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
            case shell_node_type::SNT_FUNCTION: {
                if (
                    const auto pNode = dynamic_cast<const shell_node_function *>(pRawNode);
                    pNode != nullptr
                ) {
                    if constexpr (std::is_same_v<visit_t, void>) {
                        this->visit(oSession, pNode);
                        return;
                    } else {
                        return this->visit(oSession, pNode);
                    }
                }
                break;
            }
        }

        if constexpr (std::is_same_v<visit_t, void>) {
            return;
        } else {
            return visit_t();
        }
    }
}
