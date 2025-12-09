/**
 * @file shell_tokenizer.h
 * @brief Defines class `bs::shell_tokenizer`.
 *
 * This file contains the definition of the `bs::shell_tokenizer` class,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>

#include "BashSpark/tools/fakestream.h"

namespace bs {
    /**
     * @enum shell_token_type
     * @brief Enumeration of types of tokens that can be parsed from shell commands.
     *
     * This enum categorizes various components of shell commands
     * to facilitate tokenization and parsing.
     */
    enum class shell_token_type {
        // Command sections
        TK_SPACE, ///< Represents whitespace in the command.
        TK_WORD, ///< Represents a word in the command.
        TK_UNICODE, ///< Represents Unicode characters.
        TK_ESCAPED, ///< Represents escaped characters.
        TK_QUOTE_SIMPLE, ///< Represents simple quotes (e.g., 'text').
        TK_QUOTE_DOUBLE, ///< Represents double quotes (e.g., "text").
        TK_QUOTE_BACK, ///< Represents back quotes (e.g., `command`).
        TK_DOLLAR, ///< Represents a dollar sign ($).
        TK_DOLLAR_SPECIAL, ///< Represents special dollar sign usages (e.g., $?, $).
        TK_EXCLAMATION, ///< Represents an exclamation mark (!).
        TK_CMD_SEPARATOR, ///< Represents command separators (e.g., ;).
        // Brackets
        TK_OPEN_PARENTHESIS, ///< Represents an open parenthesis ('(').
        TK_CLOSE_PARENTHESIS, ///< Represents a close parenthesis (')').
        TK_OPEN_BRACKETS, ///< Represents an open bracket ('{').
        TK_CLOSE_BRACKETS, ///< Represents a close bracket ('}').
        TK_OPEN_SQR_BRACKETS, ///< Represents an open square bracket ('[').
        TK_CLOSE_SQR_BRACKETS, ///< Represents a close square bracket (']').
        // Operators
        TK_PIPE, ///< Represents a pipe (|).
        TK_OR, ///< Represents a logical OR (||).
        TK_BACKGROUND, ///< Represents background execution (&).
        TK_AND, ///< Represents logical AND (&&).
        TK_OPERATOR, ///< Represents various operators.
        TK_EOF, ///< Represents the end of the file/input.
    };

    /**
     * @struct shell_token
     *
     * @struct shell_token
     * @brief Represents a token generated during command parsing.
     *
     * This struct holds information about the type, position,
     * and text of a token extracted from a shell command.
     */
    struct shell_token {
        /// The type of the token.
        shell_token_type m_nType;
        /// The position of the token in the input.
        std::size_t m_nPos;
        /// The text of the token.
        std::string_view m_sTokenText;
    };

    /**
     * @warning Use with caution, not foolproof
     */
    class shell_tokenizer {
    public:
        /**
         * @brief Tokenizes input from the standard input stream.
         * @param oStdIn The input fake stream to read commands from.
         * @return A vector of shell_token instances representing the parsed tokens.
         * @throw shell_exception If a syntax error is detected
         *
         * This method reads from the provided input stream and generates
         * tokens until the end of the input is reached.
         */
        static std::vector<shell_token> tokens(ifakestream &oStdIn);

    private:
        /**
           * @brief Helper method for tokenizing based on a specified delimiter.
           * @param vTokens The vector to store the parsed tokens.
           * @param oStdIn The input stream to read commands from.
           * @param cDelimiter The delimiter to use for token separation or '\0' for no delimiter
           * @throw shell_exception If a syntax error is detected
           */
        static void tokens(
            std::vector<shell_token> &vTokens,
            ifakestream &oStdIn,
            char cDelimiter
        );

        /**
         * @brief Handles tokenization for dollar sign usage.
         * @param vTokens The vector to store the parsed tokens.
         * @param oStdIn The input stream to read commands from.
         * @throw shell_exception If a syntax error is detected
         *
         * This method specifically processes tokens that contain
         * dollar sign syntax, such as variable references.
         */
        static void tokens_dollar(std::vector<shell_token> &vTokens, ifakestream &oStdIn);

        /**
         * @brief Tokenizes variables prefixed by a dollar sign.
         * @param vTokens The vector to store the parsed tokens.
         * @param oStdIn The input stream to read commands from.
         * @throw shell_exception If a syntax error is detected
         */
        static void tokens_dollar_variable(std::vector<shell_token> &vTokens, ifakestream &oStdIn);

        /**
         * @brief Tokenizes single-quoted strings in input.
         * @param vTokens The vector to store the parsed tokens.
         * @param oStdIn The input stream to read commands from.
         * @throw shell_exception If a syntax error is detected
         */
        static void tokens_quote_simple(std::vector<shell_token> &vTokens, ifakestream &oStdIn);

        /**
         * @brief Tokenizes double-quoted strings in input.
         * @param vTokens The vector to store the parsed tokens.
         * @param oStdIn The input stream to read commands from.
         * @throw shell_exception If a syntax error is detected
         */
        static void tokens_quote_double(std::vector<shell_token> &vTokens, ifakestream &oStdIn);

        /**
         * @brief Handles tokenization for backslash-escaped characters.
         * @param vTokens The vector to store the parsed tokens.
         * @param oStdIn The input stream to read commands from.
         * @throw shell_exception If a syntax error is detected
         */
        static void tokens_backslash(std::vector<shell_token> &vTokens, ifakestream &oStdIn);
    };
}
