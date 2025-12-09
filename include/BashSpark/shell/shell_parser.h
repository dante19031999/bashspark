/**
 * @file shell_parser.h
 * @brief Defines class `bs::shell_parser`.
 *
 * This file contains the definition of the `bs::shell_parser` class,
 * which provides helper methods for class `bs::shell`.
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

#include "BashSpark/shell/shell_tokenizer.h"
#include "BashSpark/shell.h"
#include "BashSpark/shell/shell_node.h"
#include "BashSpark/shell/shell_keyword.h"
#include "BashSpark/tools/fakestream.h"

namespace bs {
    class token_holder;

    /**
     * @class shell_parser
     * @brief Parses tokens into an evaluable shell AST.
     * @throw shell_parser_exception If a syntax error is found.
     *
     * The parser consumes tokens produced by `shell_tokenizer` and constructs
     * a hierarchy of `shell_node` objects representing shell expressions,
     * commands, conditionals, loops, and expansions.
     *
     * It is an internal helper for `bs::shell` and not intended for direct
     * interaction by users.
     */
    class shell_parser {
    public:
        /// Unique pointer to an expandable shell node (sugar syntax).
        using expandable_ptr = std::unique_ptr<shell_node_expandable>;
        /// Unique pointer to an evaluable shell node (sugar syntax).
        using evaluable_ptr = std::unique_ptr<shell_node_evaluable>;

    public:
        /// Maximum recursion depth allowed for nested constructs.
        constexpr static std::size_t MAX_DEPTH = shell::MAX_DEPTH;

        /**
         * @brief Parse input into an evaluable AST.
         *
         * Reads tokens from the given input stream and produces the root node
         * of the parsed command or expression.
         *
         * @param oIstream The input token stream.
         * @return The root evaluable AST node.
         */
        static evaluable_ptr parse(
            ifakestream &oIstream
        );

    private:
        shell_parser(
            ifakestream &oIstream,
            token_holder &oTokens
        );

    private:
        /**
         * @brief Parse a bare word token.
         *
         * @param oTokens Token sequence.
         * @return An expandable AST node.
         */
        [[nodiscard]] static expandable_ptr parse_word(const token_holder &oTokens);

        /**
         * @brief Parse a unicode-escaped sequence.
         *
         * @return An expandable AST node.
         */
        [[nodiscard]] expandable_ptr parse_unicode() const;

        /**
         * @brief Parse a simple single-quoted string.
         *
         * @return An expandable AST node.
         */
        [[nodiscard]] expandable_ptr parse_quote_simple() const;

        /**
         * @brief Parse a double-quoted string.
         *
         * @return An expandable AST node.
         */
        [[nodiscard]] expandable_ptr parse_quote_double();

        /**
         * @brief Parse a backtick-quoted command substitution.
         *
         * @return An expandable AST node.
         */
        [[nodiscard]] expandable_ptr parse_quote_back();

        /**
         * @brief Parse a `$` expansion (variable, command, positional, etc.).
         *
         * @return An expandable AST node.
         */
        [[nodiscard]] expandable_ptr parse_dollar();

        /**
         * @brief Parse a `$var` variable expansion.
         *
         * @return An expandable AST node.
         */
        [[nodiscard]] expandable_ptr parse_dollar_variable() const;

        /**
         * @brief Parse a `$(...)` command substitution.
         *
         * @return An expandable AST node.
         */
        [[nodiscard]] expandable_ptr parse_dollar_command();

        /**
         * @brief Parse a simple command.
         *
         * @param nMode Parsing mode.
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_command(parse_mode nMode);

        /**
         * @brief Parse a command expression (pipeline, logic operators, etc.).
         *
         * @param nMode Parsing mode.
         * @return A command expression node.
         */
        [[nodiscard]] std::unique_ptr<shell_node_command_expression>
        parse_command_expression(parse_mode nMode);

        /**
         * @brief Parse a group of commands.
         *
         * @param nMode Parsing mode.
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_command_group(
            parse_mode nMode
        );

        /**
         * @brief Parse an operator inside a command group.
         *
         * @param vExpressions Accumulated command expressions.
         * @param nNodeType The operator node type to create.
         * @param nPos The token position.
         * @param nMode Parsing mode.
         */
        void parse_command_group_oper(
            std::vector<evaluable_ptr> &vExpressions,
            shell_node_type nNodeType,
            std::size_t nPos,
            parse_mode nMode
        );

        /**
         * @brief Parse a parenthesized subshell expression.
         *
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_parentheses();

        /**
         * @brief Parse a `{ ... }` block.
         *
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_brackets();

        /**
         * @brief Parse a `[ ... ]` test expression.
         *
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_sqr_brackets();

        /**
         * @brief Parse a general block until an ending token.
         *
         * @param nEnd Ending token type.
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_block(shell_token_type nEnd);

        /**
         * @brief Parse a block terminated by a keyword.
         *
         * @param nEnd Ending keyword.
         * @param nMode Parsing mode.
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_block(shell_keyword nEnd, parse_mode nMode);

        /**
         * @brief Parse a specific keyword construct (`if`, `for`, etc.).
         *
         * @param nKeyword The keyword encountered.
         * @param nMode Parsing mode.
         */
        [[nodiscard]] evaluable_ptr parse_keyword(shell_keyword nKeyword, parse_mode nMode);

        /**
         * @brief Parse a `test` conditional expression.
         *
         * @return An expandable AST node.
         */
        [[nodiscard]] expandable_ptr parse_test_expression();

        /**
         * @brief Parse an `if` statement.
         *
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_if();

        /**
         * @brief Parse a `for` loop.
         *
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_for();

        /**
         * @brief Parse a `while` loop.
         *
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_while();

        /**
         * @brief Parse an `until` loop.
         *
         * @return An evaluable AST node.
         */
        [[nodiscard]] evaluable_ptr parse_until();

        /**
         * @brief Parse `function`.
         *
         * @return An evaluable AST node.
         */
        shell_parser::evaluable_ptr parse_function();

    public:
        /**
         * @brief Increase the nesting depth and check limits.
         *
         * @param nPos Position of the construct causing the depth increase.
         */
        void increase_depth(std::size_t nPos);

        /**
         * @brief Decrease the current nesting depth.
         */
        void decrease_depth();

    private:
        /// Current recursion depth.
        std::size_t m_nDepth = 0;
        /// Istream to parse
        ifakestream &m_oIstream;
        // Token holder
        token_holder &m_oTokens;
    };
}
