/**
 * @file shell_tokenizer.cpp
 * @brief Implements class `bs::shell_tokenizer`.
 *
 * This file contains the implementation of the `bs::shell_tokenizer` class,
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

#include "BashSpark/shell/shell_tokenizer.h"

#include "BashSpark/tools/utf.h"
#include "BashSpark/shell/shell_parser_exception.h"

namespace bs {
    namespace {
        void add_word(
            std::vector<shell_token> &vTokens,
            const ifakestream &oStdIn,
            const std::size_t nBegin
        ) {
            const auto nPos = oStdIn.tell() - 1;
            if (vTokens.size() > nBegin && vTokens.back().m_nType == shell_token_type::TK_WORD) {
                // Extend existing word
                vTokens.back().m_sTokenText = {
                    vTokens.back().m_sTokenText.data(),
                    vTokens.back().m_sTokenText.length() + 1
                };
            } else {
                // Create new word
                vTokens.emplace_back(
                    shell_token_type::TK_WORD, nPos,
                    oStdIn.sub_view(nPos, 1)
                );
            }
        }

        void add_space(
            std::vector<shell_token> &vTokens,
            const ifakestream &oStdIn
        ) {
            const auto nPos = oStdIn.tell() - 1;
            if (vTokens.empty() || vTokens.back().m_nType != shell_token_type::TK_SPACE) {
                // Create new word
                vTokens.emplace_back(
                    shell_token_type::TK_SPACE, nPos,
                    oStdIn.sub_view(nPos, 1)
                );
            } else {
                // Extend existing word
                vTokens.back().m_sTokenText = {
                    vTokens.back().m_sTokenText.data(),
                    vTokens.back().m_sTokenText.length() + 1
                };
            }
        }

        void add_space_s2(
            std::vector<shell_token> &vTokens,
            const ifakestream &oStdIn
        ) {
            const auto nPos = oStdIn.tell() - 1;
            if (vTokens.empty() || vTokens.back().m_nType != shell_token_type::TK_SPACE) {
                // Create new word
                vTokens.emplace_back(
                    shell_token_type::TK_SPACE, nPos,
                    oStdIn.sub_view(nPos, 2)
                );
            } else {
                // Extend existing word
                vTokens.back().m_sTokenText = {
                    vTokens.back().m_sTokenText.data(),
                    vTokens.back().m_sTokenText.length() + 2
                };
            }
        }

        constexpr shell_token_type get_token_type(const char cChar) {
            switch (cChar) {
                case '$':
                    return shell_token_type::TK_DOLLAR;
                case '(':
                    return shell_token_type::TK_OPEN_PARENTHESIS;
                case ')':
                    return shell_token_type::TK_CLOSE_PARENTHESIS;
                case '{':
                    return shell_token_type::TK_OPEN_BRACKETS;
                case '}':
                    return shell_token_type::TK_CLOSE_BRACKETS;
                case '[':
                    return shell_token_type::TK_OPEN_SQR_BRACKETS;
                case ']':
                    return shell_token_type::TK_CLOSE_SQR_BRACKETS;
                case '\'':
                    return shell_token_type::TK_QUOTE_SIMPLE;
                case '\"':
                    return shell_token_type::TK_QUOTE_DOUBLE;
                case '`':
                    return shell_token_type::TK_QUOTE_BACK;
                case '\\':
                    return shell_token_type::TK_ESCAPED;
                case ' ':
                case '\t':
                    return shell_token_type::TK_SPACE;
                case '\n':
                case ';':
                    return shell_token_type::TK_CMD_SEPARATOR;
                case '|':
                    return shell_token_type::TK_PIPE;
                case '&':
                    return shell_token_type::TK_BACKGROUND;
                default:
                    return shell_token_type::TK_WORD;
            }
        }

        constexpr shell_token_type get_token_type_quote_complex(
            const char cChar
        ) {
            switch (cChar) {
                case '$':
                    return shell_token_type::TK_DOLLAR;
                case '\"':
                    return shell_token_type::TK_QUOTE_DOUBLE;
                case '`':
                    return shell_token_type::TK_QUOTE_BACK;
                case '\\':
                    return shell_token_type::TK_ESCAPED;
                default:
                    return shell_token_type::TK_WORD;
            }
        }
    }

    std::vector<shell_token> shell_tokenizer::tokens(ifakestream &oStdIn) {
        std::vector<shell_token> vTokens;
        // Covers most commands and does not make a dent on RAM
        vTokens.reserve(64);
        tokens(vTokens, oStdIn, '\0');
#ifdef BS_DEBUG
        std::cout << "txt " << oStdIn.view() << std::endl;
        std::cout << "tokens ";
        for (auto &oToken: vTokens)
            std::cout << "<" << oToken.m_sTokenText << "> ";
        std::cout << std::endl;
#endif
        return vTokens;
    }

    void shell_tokenizer::tokens(
        std::vector<shell_token> &vTokens,
        ifakestream &oStdIn,
        const char cDelimiter
    ) {
        const auto nStartPos = oStdIn.tell();
        auto nBegin = vTokens.size();
        auto cChar = oStdIn.get();

        while (cChar != ifakestream::EOF_VALUE && cChar != cDelimiter) {
            // Get pos
            std::size_t nPos = oStdIn.tell() - 1;

            // Sub-parsing
            switch (
                const auto nTokenType = get_token_type(static_cast<char>(cChar))
            ) {
                case shell_token_type::TK_QUOTE_SIMPLE: {
                    tokens_quote_simple(vTokens, oStdIn);
                    break;
                }

                case shell_token_type::TK_QUOTE_DOUBLE: {
                    // Add quote
                    tokens_quote_double(vTokens, oStdIn);
                    break;
                }

                case shell_token_type::TK_QUOTE_BACK: {
                    vTokens.emplace_back(
                        nTokenType, nPos,
                        oStdIn.sub_view(nPos, 1)
                    );
                    tokens(vTokens, oStdIn, '`');
                    nBegin = vTokens.size();
                    break;
                }

                case shell_token_type::TK_ESCAPED: {
                    tokens_backslash(vTokens, oStdIn);
                    nBegin = vTokens.size();
                    break;
                }

                case shell_token_type::TK_DOLLAR: {
                    tokens_dollar(vTokens, oStdIn);
                    nBegin = vTokens.size();
                    break;
                }

                case shell_token_type::TK_WORD: {
                    add_word(vTokens, oStdIn, nBegin);
                    break;
                }

                case shell_token_type::TK_SPACE: {
                    add_space(vTokens, oStdIn);
                    break;
                }

                case shell_token_type::TK_PIPE: {
                    if (oStdIn.peek() == '|') {
                        oStdIn.get();
                        vTokens.emplace_back(
                            shell_token_type::TK_OR, nPos,
                            oStdIn.sub_view(nPos, 2)
                        );
                    } else {
                        vTokens.emplace_back(
                            shell_token_type::TK_PIPE, nPos,
                            oStdIn.sub_view(nPos, 1)
                        );
                    }
                    break;
                }

                case shell_token_type::TK_BACKGROUND: {
                    if (oStdIn.peek() == '&') {
                        oStdIn.get();
                        vTokens.emplace_back(
                            shell_token_type::TK_AND, nPos,
                            oStdIn.sub_view(nPos, 2)
                        );
                    } else {
                        vTokens.emplace_back(
                            shell_token_type::TK_BACKGROUND, nPos,
                            oStdIn.sub_view(nPos, 1)
                        );
                    }
                    break;
                }

                case shell_token_type::TK_OPEN_PARENTHESIS: {
                    vTokens.emplace_back(
                        nTokenType, nPos,
                        oStdIn.sub_view(nPos, 1)
                    );
                    tokens(vTokens, oStdIn, ')');
                    nBegin = vTokens.size();
                    break;
                }

                case shell_token_type::TK_OPEN_BRACKETS: {
                    vTokens.emplace_back(
                        nTokenType, nPos,
                        oStdIn.sub_view(nPos, 1)
                    );
                    tokens(vTokens, oStdIn, '}');
                    nBegin = vTokens.size();
                    break;
                }

                case shell_token_type::TK_OPEN_SQR_BRACKETS: {
                    vTokens.emplace_back(
                        nTokenType, nPos,
                        oStdIn.sub_view(nPos, 1)
                    );
                    tokens(vTokens, oStdIn, ']');
                    nBegin = vTokens.size();
                    break;
                }

                case shell_token_type::TK_CLOSE_PARENTHESIS:
                case shell_token_type::TK_CLOSE_BRACKETS:
                case shell_token_type::TK_CLOSE_SQR_BRACKETS: {
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        oStdIn.str(), nPos
                    };
                }

                default: {
                    vTokens.emplace_back(
                        nTokenType, nPos,
                        oStdIn.sub_view(nPos, 1)
                    );
                    break;
                }
            }

            cChar = oStdIn.get();
        }

        if (cDelimiter != '\0') {
            if (cChar == cDelimiter) {
                auto nPos = oStdIn.tell() - 1;
                vTokens.emplace_back(
                    get_token_type(cDelimiter), nPos,
                    oStdIn.sub_view(nPos, 1)
                );
            } else {
                auto nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR;
                if (cDelimiter == ')')nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_PARENTHESES;
                if (cDelimiter == ']')nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SQR_BRACKETS;
                if (cDelimiter == '}')nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_BRACKETS;
                if (cDelimiter == '`')nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_BACK_QUOTES;
                throw shell_parser_exception{nStatus, oStdIn.str(), nStartPos};
            }
        }
    }

    void shell_tokenizer::tokens_quote_simple(
        std::vector<shell_token> &vTokens,
        ifakestream &oStdIn
    ) {
        std::size_t nQuotePos = oStdIn.tell() - 1;
        const auto nBegin = vTokens.size();
        auto cChar = oStdIn.get();

        // Add quote
        vTokens.emplace_back(
            shell_token_type::TK_QUOTE_SIMPLE, nQuotePos,
            oStdIn.sub_view(nQuotePos, 1)
        );

        // Parse quotes
        while (cChar != '\'' && cChar != ifakestream::EOF_VALUE) {
            if (cChar == '\\') {
                tokens_backslash(vTokens, oStdIn);
            } else {
                add_word(vTokens, oStdIn, nBegin);
            }

            cChar = oStdIn.get();
        }

        // Except unclosed quotes
        if (cChar == ifakestream::EOF_VALUE) {
            constexpr auto nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SIMPLE_QUOTES;
            throw shell_parser_exception{
                nStatus, oStdIn.str(), nQuotePos
            };
        }

        // Add quote
        nQuotePos = oStdIn.tell() - 1;
        vTokens.emplace_back(
            shell_token_type::TK_QUOTE_SIMPLE, nQuotePos,
            oStdIn.sub_view(nQuotePos, 1)
        );
    }

    void shell_tokenizer::tokens_quote_double(
        std::vector<shell_token> &vTokens,
        ifakestream &oStdIn
    ) {
        const std::size_t nQuotePos = oStdIn.tell();
        auto nBegin = vTokens.size();
        auto cChar = oStdIn.get();

        auto nPos = oStdIn.tell() - 1;
        vTokens.emplace_back(
            shell_token_type::TK_QUOTE_DOUBLE, nPos,
            oStdIn.sub_view(nPos, 1)
        );

        while (cChar != '\"' && cChar != ifakestream::EOF_VALUE) {
            // Sub-parsing
            switch (
                auto nTokenType = get_token_type_quote_complex(static_cast<char>(cChar))
            ) {
                case shell_token_type::TK_QUOTE_BACK: {
                    vTokens.emplace_back(
                        nTokenType, nPos,
                        oStdIn.sub_view(nPos, 1)
                    );
                    tokens(vTokens, oStdIn, '`');
                    nBegin = vTokens.size();
                    break;
                }
                case shell_token_type::TK_ESCAPED: {
                    tokens_backslash(vTokens, oStdIn);
                    nBegin = vTokens.size();
                    break;
                }
                case shell_token_type::TK_DOLLAR: {
                    tokens_dollar(vTokens, oStdIn);
                    nBegin = vTokens.size();
                    break;
                }
                case shell_token_type::TK_WORD: {
                    add_word(vTokens, oStdIn, nBegin);
                    break;
                }
                default: break;
            }

            cChar = oStdIn.get();
        }

        // Except unclosed quotes
        if (cChar == ifakestream::EOF_VALUE) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_DOUBLE_QUOTES,
                oStdIn.str(), nQuotePos
            };
        }

        nPos = oStdIn.tell() - 1;
        vTokens.emplace_back(
            shell_token_type::TK_QUOTE_DOUBLE, nPos,
            oStdIn.sub_view(nPos, 1)
        );
    }

    void shell_tokenizer::tokens_dollar(std::vector<shell_token> &vTokens, ifakestream &oStdIn) {
        // Add dollar
        auto nDollarPos = oStdIn.tell() - 1;
        vTokens.emplace_back(
            shell_token_type::TK_DOLLAR, nDollarPos,
            oStdIn.sub_view(nDollarPos, 1)
        );

        // Process next char
        auto cChar = oStdIn.get();
        auto nPos = oStdIn.tell() - 1;

        if (cChar == '0' || cChar == '$' || cChar == '#' ||
            cChar == '@' || cChar == '?') {
            // Add special variable identifier
            vTokens.emplace_back(
                shell_token_type::TK_DOLLAR_SPECIAL, nPos,
                oStdIn.sub_view(nPos, 1)
            );
        } else if ('1' <= cChar && cChar <= '9') {
            // Add arg number
            vTokens.emplace_back(
                shell_token_type::TK_WORD, nPos,
                oStdIn.sub_view(nPos, 1)
            );
        } else if (cChar == '{') {
            tokens_dollar_variable(vTokens, oStdIn);
        } else if (cChar == '(') {
            nPos = oStdIn.tell() - 1;
            vTokens.emplace_back(
                shell_token_type::TK_OPEN_PARENTHESIS, nPos,
                oStdIn.sub_view(nPos, 1)
            );
            tokens(vTokens, oStdIn, ')');
        } else if (
            cChar == '_'
            || 'A' <= cChar && cChar <= 'Z'
            || 'a' <= cChar && cChar <= 'z'
        ) {
            // Process variables
            cChar = oStdIn.get();
            while (
                cChar == '_'
                || 'A' <= cChar && cChar <= 'Z'
                || 'a' <= cChar && cChar <= 'z'
                || '0' <= cChar && cChar <= '9'
            ) {
                cChar = oStdIn.get();
            }

            // Add variable name
            oStdIn.put_back();
            vTokens.emplace_back(
                shell_token_type::TK_WORD, nPos,
                oStdIn.sub_view(nPos, oStdIn.tell() - nPos)
            );
        } else {
            // Undo dollar and re-append as word
            vTokens.pop_back();
            oStdIn.put_back();
            add_word(vTokens, oStdIn, vTokens.size());
        }
    }

    void shell_tokenizer::tokens_dollar_variable(std::vector<shell_token> &vTokens, ifakestream &oStdIn) {
        // Add brace
        const auto nBracketPos = oStdIn.tell() - 1;
        vTokens.emplace_back(
            shell_token_type::TK_OPEN_BRACKETS, nBracketPos,
            oStdIn.sub_view(nBracketPos, 1)
        );

        // Process next char
        auto cChar = oStdIn.get();
        auto nVariableStartPos = oStdIn.tell() - 1;

        // Check double hop
        if (cChar == '!') {
            auto nPos = oStdIn.tell();
            vTokens.emplace_back(
                shell_token_type::TK_EXCLAMATION, nPos,
                oStdIn.sub_view(nPos, 1)
            );
            cChar = oStdIn.get();
            nVariableStartPos = oStdIn.tell() - 1;
        }

        if ('1' <= cChar && cChar <= '9') {
            // Process args
            cChar = oStdIn.get();
            while ('0' <= cChar && cChar <= '9') {
                cChar = oStdIn.get();
            }
        } else if (
            cChar == '_'
            || 'A' <= cChar && cChar <= 'Z'
            || 'a' <= cChar && cChar <= 'z'
        ) {
            // Process variables
            cChar = oStdIn.get();
            while (
                cChar == '_'
                || 'A' <= cChar && cChar <= 'Z'
                || 'a' <= cChar && cChar <= 'z'
                || '0' <= cChar && cChar <= '9'
            ) {
                cChar = oStdIn.get();
            }
        } else {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_VARIABLE_NAME,
                oStdIn.str(), nBracketPos,
            };
        }

        // Check closed variable
        // Check variable name is valid
        if (cChar != '}') {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_VARIABLE,
                oStdIn.str(), nBracketPos,
            };
        }

        // Add variable name
        auto nPos = oStdIn.tell() - 1;
        vTokens.emplace_back(
            shell_token_type::TK_WORD, nVariableStartPos,
            oStdIn.sub_view(nVariableStartPos, nPos - nVariableStartPos)
        );
        // Add brace
        vTokens.emplace_back(
            shell_token_type::TK_CLOSE_BRACKETS, nPos,
            oStdIn.sub_view(nPos, 1)
        );
    }


    void shell_tokenizer::tokens_backslash(std::vector<shell_token> &vTokens, ifakestream &oStdIn) {
        const auto nPos = oStdIn.tell() - 1;
        bool bFailure = false;

        switch (
            const auto cChar = oStdIn.get();
            cChar
        ) {
            case ' ':
            case 'n':
            case 't':
            case '\\':
            case '\'':
            case '\"':
            case '`':
            case '$':
            case '|':
            case '&':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}': {
                vTokens.emplace_back(
                    shell_token_type::TK_ESCAPED, nPos,
                    oStdIn.sub_view(nPos, 2)
                );
                break;
            }
            case '\n': {
                add_space_s2(vTokens, oStdIn);
                break;
            }
            case 'x': {
                if (char32_t cChar32; parse_utf(oStdIn, 1, cChar32)) {
                    vTokens.emplace_back(
                        shell_token_type::TK_UNICODE, nPos,
                        oStdIn.sub_view(nPos, 2 + 2)
                    );
                } else bFailure = true;
                break;
            }
            case 'u': {
                if (char32_t cChar32; parse_utf(oStdIn, 2, cChar32)) {
                    const auto nLength = cChar32 > 0xFFFF && cChar32 <= 0x10FFFF
                                             ? 2 + 4 + 2 + 4
                                             : 2 + 4;
                    vTokens.emplace_back(
                        shell_token_type::TK_UNICODE, nPos,
                        oStdIn.sub_view(nPos, nLength)
                    );
                } else bFailure = true;
                break;
            }
            case 'U': {
                if (char32_t cChar32; parse_utf(oStdIn, 4, cChar32)) {
                    vTokens.emplace_back(
                        shell_token_type::TK_UNICODE, nPos,
                        oStdIn.sub_view(nPos, 2 + 8)
                    );
                } else bFailure = true;
                break;
            }
            default: break;
        }

        if (bFailure) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_BAD_ENCODING,
                oStdIn.str(), nPos
            };
        }
    }
}
