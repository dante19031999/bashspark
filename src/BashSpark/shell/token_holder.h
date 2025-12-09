/**
 * @file token_holder.h
 * @brief Defines class `bs::token_holder`.
 *
 * This file contains the definition of the `bs::token_holder` class,
 * which provides helper methods for class `bs::shell_parser`.
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

#include <cstdint>

#include "BashSpark/shell/shell_keyword.h"
#include "BashSpark/shell/shell_tokenizer.h"

namespace bs {
    /**
     * @class token_holder
     * @brief A class that manages a collection of shell tokens for processing.
     *
     * The `bs::token_holder` class stores a list of shell tokens and provides functionalities
     * to navigate through them, including retrieving the current, next, and previous tokens,
     * as well as checking their types and positions. It utilizes an `bs::ifakestream` for
     * string representation and size.
     */
    class token_holder {
    public:
        /**
         * @brief Constructs a token_holder instance.
         *
         * @param oIstream Reference to an ifakestream for input handling.
         * @param vTokens Vector containing shell tokens to manage.
         */
        token_holder(
            ifakestream &oIstream,
            std::vector<shell_token> vTokens
        ) : m_oIstream(oIstream),
            m_vTokens(std::move(vTokens)) {
        }

        /**
         * @brief Retrieves the current token.
         *
         * @return A pointer to the current shell_token, or nullptr if out of bounds.
         */
        [[nodiscard]] const shell_token *current() const noexcept {
            if (m_nPos >= m_vTokens.size() || m_nPos < 0) return nullptr;
            return &m_vTokens[m_nPos];
        }

        /**
         * @brief Retrieves the next token.
         *
         * @return A pointer to the next shell_token, or nullptr if out of bounds.
         */
        [[nodiscard]] const shell_token *next() const noexcept {
            if (m_nPos + 1 >= m_vTokens.size() || m_nPos + 1 < 0) return nullptr;
            return &m_vTokens[m_nPos + 1];
        }

        /**
         * @brief Retrieves the previous token.
         *
         * @return A pointer to the previous shell_token, or nullptr if out of bounds.
         */
        [[nodiscard]] const shell_token *previous() const noexcept {
            if (m_nPos >= m_vTokens.size() + 1 || m_nPos <= 0) return nullptr;
            return &m_vTokens[m_nPos - 1];
        }

        /**
         * @brief Retrieves and advances to the next token.
         *
         * @return A pointer to the new current shell_token, or nullptr if out of bounds.
         */
        const shell_token *get() noexcept {
            ++m_nPos;
            if (m_nPos >= m_vTokens.size()) return nullptr;
            return &m_vTokens[m_nPos];
        }

        /**
         * @brief Moves the position back by one token.
         *
         * This function allows for retroactive examination of the last token retrieved.
         */
        void put_back() noexcept {
            --m_nPos;
        }

        /**
         * @brief Retrieves the current position of the token holder.
         *
         * @return The current position as an std::size_t. If out of bounds, returns the size of the input stream.
         */
        [[nodiscard]] std::size_t pos() const noexcept {
            if (m_nPos >= m_vTokens.size() || m_nPos < 0) return m_oIstream.size();
            return m_nPos;
        }

        /**
         * @brief Checks if the current token is of a specified type.
         *
         * @param nType The type to check against.
         * @return True if the current token matches the type; otherwise, false.
         */
        [[nodiscard]] bool is(const shell_token_type nType) const noexcept {
            if (m_nPos >= m_vTokens.size() || m_nPos < 0) return false;
            return m_vTokens[m_nPos].m_nType == nType;
        }

        /**
         * @brief Checks if the next token is of a specified type.
         *
         * @param nType The type to check against.
         * @return True if the next token matches the type; otherwise, false.
         */
        [[nodiscard]] bool is_next(const shell_token_type nType) const noexcept {
            if (m_nPos + 1 >= m_vTokens.size()) return false;
            return m_vTokens[m_nPos + 1].m_nType == nType;
        }

        /**
         * @brief Retrieves the current shell keyword.
         *
         * @return The shell keyword associated with the current token.
         */
        [[nodiscard]] shell_keyword keyword() const noexcept;

        /**
         * @brief Checks if the current token is a specific keyword.
         *
         * @param nKeyword The keyword to compare against.
         * @return True if the current token matches the keyword; otherwise, false.
         */
        [[nodiscard]] bool keyword(const shell_keyword nKeyword) const noexcept {
            return this->keyword() == nKeyword;
        }

    private:
        /// Reference to the input stream for string operations.
        ifakestream &m_oIstream;
        /// Vector of shell tokens being managed.
        std::vector<shell_token> m_vTokens;
        /// Current position index in the token vector.
        std::intptr_t m_nPos = -1;
    };

    inline shell_keyword token_holder::keyword() const noexcept {
        // Get current and next
        const auto pCurrent = this->current();
        if (pCurrent == nullptr) return shell_keyword::SK_NONE;
        const auto pNext = this->next();

        // Finish if next is null
        if (pNext == nullptr) {
            if (pCurrent->m_nType == shell_token_type::TK_WORD)
                return get_keyword_id(pCurrent->m_sTokenText);
            return shell_keyword::SK_NONE;
        }

        // Check next token
        if (
            const auto nTypeNext = pNext->m_nType;
            nTypeNext != shell_token_type::TK_SPACE
            && nTypeNext != shell_token_type::TK_CMD_SEPARATOR
            && nTypeNext != shell_token_type::TK_OPEN_PARENTHESIS
            && nTypeNext != shell_token_type::TK_OPEN_BRACKETS
            && nTypeNext != shell_token_type::TK_OPEN_SQR_BRACKETS
            && nTypeNext != shell_token_type::TK_CLOSE_PARENTHESIS
            && nTypeNext != shell_token_type::TK_CLOSE_BRACKETS
            && nTypeNext != shell_token_type::TK_CLOSE_SQR_BRACKETS
        ) {
            return shell_keyword::SK_NONE;
        }

        if (pCurrent->m_nType == shell_token_type::TK_WORD)
            return get_keyword_id(pCurrent->m_sTokenText);
        return shell_keyword::SK_NONE;
    }
}
