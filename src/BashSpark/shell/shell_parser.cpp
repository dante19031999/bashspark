/**
 * @file shell_parser.cpp
 * @brief Implements class `bs::shell_parser`.
 *
 * This file contains the implementation of the `bs::shell_parser` class,
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

#include "BashSpark/shell/shell_parser.h"

#include <memory>
#include <ranges>

#include "shell_tools.h"
#include "token_holder.h"
#include "BashSpark/shell/shell_node_visitor_json.h"

#include "BashSpark/shell/shell_tokenizer.h"
#include "BashSpark/shell/shell_parser_exception.h"
#include "BashSpark/shell/shell_status.h"
#include "BashSpark/tools/shell_def.h"

namespace bs {
    /// Unique pointer to evaluable node (sugar syntax).
    using evaluable_ptr = shell_parser::evaluable_ptr;
    /// Unique pointer to expandable node (sugar syntax).
    using expandable_ptr = shell_parser::expandable_ptr;

    namespace {
        std::uint64_t get_arg_number(const token_holder &m_oTokens) {
            const std::string sArg(m_oTokens.current()->m_sTokenText);
            return std::stoull(sArg);
        }

        class depth_guard {
        public:
            depth_guard(shell_parser *pParser, const std::size_t nPos)
                : m_pParser(pParser) {
                this->m_pParser->increase_depth(nPos);
            }

            ~depth_guard() {
                this->m_pParser->decrease_depth();
            }

        private:
            shell_parser *m_pParser;
        };
    }

    shell_parser::shell_parser(
        ifakestream &oIstream,
        token_holder &oTokens
    )
        : m_oIstream(oIstream),
          m_oTokens(oTokens) {
    }

    evaluable_ptr shell_parser::parse(
        ifakestream &oIstream
    ) {
        // Tokenize
        token_holder oTokens = {
            oIstream,
            shell_tokenizer::tokens(oIstream)
        };

        // Parse
        const std::unique_ptr<shell_parser> pParser(new shell_parser(
            oIstream,
            oTokens
        ));
        auto pEvaluable = pParser->parse_block(
            shell_token_type::TK_EOF
        );

        // Return
        if (pEvaluable == nullptr)
            return std::make_unique<shell_node_null_command>(0);
        return pEvaluable;
    }

    void shell_parser::increase_depth(std::size_t nPos) {
        this->m_nDepth++;
        if (this->m_nDepth > MAX_DEPTH) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_MAX_DEPTH_REACHED,
                m_oIstream.str(), nPos
            };
        }
    }

    void shell_parser::decrease_depth() {
        if (this->m_nDepth > 0) this->m_nDepth--;
    }

    expandable_ptr shell_parser::parse_word(const token_holder &oTokens) {
        const auto pToken = oTokens.current();
        return std::make_unique<shell_node_word>(
            pToken->m_nPos,
            std::string{pToken->m_sTokenText}
        );
    }

    expandable_ptr shell_parser::parse_unicode() const {
        const auto pToken = m_oTokens.current();
        if (pToken->m_sTokenText.length() < 2) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_BAD_ENCODING,
                m_oIstream.str(), pToken->m_nPos
            };
        }
        switch (pToken->m_sTokenText[1]) {
            case 'n':
                return std::make_unique<shell_node_unicode>(
                    pToken->m_nPos, '\n'
                );
            case 't':
                return std::make_unique<shell_node_unicode>(
                    pToken->m_nPos, '\t'
                );
            case ' ':
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
            case '}':
                return std::make_unique<shell_node_unicode>(
                    pToken->m_nPos, pToken->m_sTokenText[1]
                );
            case 'x': {
                ifakestream oStdIn(pToken->m_sTokenText.data() + 2, pToken->m_sTokenText.length() - 2);
                if (char32_t cChar32; parse_utf(oStdIn, 1, cChar32)) {
                    return std::make_unique<shell_node_unicode>(
                        pToken->m_nPos,
                        cChar32
                    );
                }
                break;
            }
            case 'u': {
                ifakestream oStdIn(pToken->m_sTokenText.data() + 2, pToken->m_sTokenText.length() - 2);
                if (char32_t cChar32; parse_utf(oStdIn, 2, cChar32)) {
                    return std::make_unique<shell_node_unicode>(
                        pToken->m_nPos,
                        cChar32
                    );
                }
                break;
            }
            case 'U': {
                ifakestream oStdIn(pToken->m_sTokenText.data() + 2, pToken->m_sTokenText.length() - 2);
                if (char32_t cChar32; parse_utf(oStdIn, 4, cChar32)) {
                    return std::make_unique<shell_node_unicode>(
                        pToken->m_nPos,
                        cChar32
                    );
                }
                break;
            }
            default: break;
        }


        throw shell_parser_exception{
            shell_status::SHELL_ERROR_BAD_ENCODING,
            m_oIstream.str(), pToken->m_nPos
        };
    }

    expandable_ptr shell_parser::parse_quote_simple() const {
        // Prepare
        const auto nStartPos = m_oTokens.pos();
        std::vector<expandable_ptr> vTokens;
        auto pToken = m_oTokens.get();

        // Fetch subtokens
        while (pToken != nullptr && pToken->m_nType != shell_token_type::TK_QUOTE_SIMPLE) {
            switch (pToken->m_nType) {
                case shell_token_type::TK_WORD:
                    vTokens.push_back(parse_word(m_oTokens));
                    break;
                case shell_token_type::TK_ESCAPED:
                case shell_token_type::TK_UNICODE:
                    vTokens.push_back(parse_unicode());
                    break;
                default: {
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
                }
            }
            pToken = m_oTokens.get();
        }

        // Check close
        if (pToken == nullptr) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SIMPLE_QUOTES,
                m_oIstream.str(), nStartPos
            };
        }

        // Build and return
        return std::make_unique<shell_node_str_simple>(
            pToken->m_nPos,
            std::move(vTokens)
        );
    }

    expandable_ptr shell_parser::parse_quote_double() {
        // Prepare
        const auto nStartPos = m_oTokens.pos();
        std::vector<expandable_ptr> vTokens;
        auto pToken = m_oTokens.get();

        // Fetch subtokens
        while (pToken != nullptr && pToken->m_nType != shell_token_type::TK_QUOTE_DOUBLE) {
            switch (pToken->m_nType) {
                case shell_token_type::TK_WORD:
                    vTokens.push_back(parse_word(m_oTokens));
                    break;
                case shell_token_type::TK_ESCAPED:
                case shell_token_type::TK_UNICODE:
                    vTokens.push_back(parse_unicode());
                    break;
                case shell_token_type::TK_DOLLAR:
                    vTokens.push_back(parse_dollar());
                    break;
                case shell_token_type::TK_QUOTE_BACK:
                    vTokens.push_back(parse_quote_back());
                    break;
                default: {
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
                }
            }
            pToken = m_oTokens.get();
        }

        // Check close
        if (pToken == nullptr) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_DOUBLE_QUOTES,
                m_oIstream.str(), nStartPos
            };
        }

        // Build and return
        return std::make_unique<shell_node_str_double>(
            pToken->m_nPos,
            std::move(vTokens)
        );
    }

    expandable_ptr shell_parser::parse_quote_back() {
        auto nPos = m_oTokens.pos();
        // Get block
        auto pBlock = parse_block(
            shell_token_type::TK_QUOTE_BACK
        );
        if (pBlock == nullptr)
            pBlock = std::make_unique<shell_node_null_command>(nPos);
        // Build node
        return std::make_unique<shell_node_str_back>(
            nPos, std::move(pBlock)
        );
    }

    expandable_ptr shell_parser::parse_dollar() {
        // Prepare
        const auto nStartPos = m_oTokens.pos();
        std::vector<expandable_ptr> vTokens;

        // Get first token
        const auto pToken = m_oTokens.get();
        if (pToken == nullptr) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), nStartPos
            };
        }

        switch (pToken->m_nType) {
            case shell_token_type::TK_WORD: {
                if (is_arg(pToken->m_sTokenText)) {
                    std::uint64_t nArg = get_arg_number(m_oTokens);
                    return std::make_unique<shell_node_arg>(pToken->m_nPos, nArg);
                }
                if (is_var(pToken->m_sTokenText)) {
                    return std::make_unique<shell_node_variable>(
                        pToken->m_nPos, std::string{pToken->m_sTokenText}
                    );
                }
                throw shell_parser_exception{
                    shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_VARIABLE_NAME,
                    m_oIstream.str(), pToken->m_nPos
                };
            }
            case shell_token_type::TK_DOLLAR_SPECIAL: {
                const char cSpecial = pToken->m_sTokenText.empty()
                                          ? '\0'
                                          : pToken->m_sTokenText[0];
                return std::make_unique<shell_node_dollar_special>(
                    pToken->m_nPos, cSpecial
                );
            }
            case shell_token_type::TK_OPEN_BRACKETS:
                return parse_dollar_variable();
            case shell_token_type::TK_OPEN_PARENTHESIS:
                return parse_dollar_command();
            default:
                throw shell_parser_exception{
                    shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                    m_oIstream.str(), pToken->m_nPos
                };
        }
    }

    expandable_ptr shell_parser::parse_dollar_variable() const {
        // Prepare
        const auto nStartPos = m_oTokens.pos();
        std::vector<expandable_ptr> vTokens;
        bool bDoubleHop = false;

        // Get first token
        auto pNameToken = m_oTokens.get();
        if (pNameToken == nullptr) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_VARIABLE,
                m_oIstream.str(), nStartPos
            };
        }

        // Check double hop
        if (pNameToken->m_nType == shell_token_type::TK_EXCLAMATION) {
            bDoubleHop = true;
            pNameToken = m_oTokens.get();
            if (pNameToken == nullptr) {
                throw shell_parser_exception{
                    shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_VARIABLE,
                    m_oIstream.str(), nStartPos
                };
            }
        }

        // Check name
        if (pNameToken->m_nType != shell_token_type::TK_WORD) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), nStartPos
            };
        }
        if (!is_arg(pNameToken->m_sTokenText) && !is_var(pNameToken->m_sTokenText)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_VARIABLE_NAME,
                m_oIstream.str(), nStartPos
            };
        }

        // Check close
        if (m_oTokens.get() == nullptr) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_VARIABLE,
                m_oIstream.str(), nStartPos
            };
        }

        // Build and return double hop
        if (bDoubleHop) {
            if (is_arg(pNameToken->m_sTokenText)) {
                auto nArg = get_arg_number(m_oTokens);
                return std::make_unique<shell_node_dollar_arg_dhop>(pNameToken->m_nPos, nArg);
            }
            if (is_var(pNameToken->m_sTokenText)) {
                return std::make_unique<shell_node_dollar_variable_dhop>(
                    pNameToken->m_nPos, std::string{pNameToken->m_sTokenText}
                );
            }
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_VARIABLE_NAME,
                m_oIstream.str(), pNameToken->m_nPos
            };
        }

        // Build and return
        if (is_arg(pNameToken->m_sTokenText)) {
            auto nArg = get_arg_number(m_oTokens);
            return std::make_unique<shell_node_dollar_arg>(pNameToken->m_nPos, nArg);
        }
        if (is_var(pNameToken->m_sTokenText)) {
            return std::make_unique<shell_node_dollar_variable>(
                pNameToken->m_nPos, std::string{pNameToken->m_sTokenText}
            );
        }
        throw shell_parser_exception{
            shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_VARIABLE_NAME,
            m_oIstream.str(), pNameToken->m_nPos
        };
    }

    expandable_ptr shell_parser::parse_dollar_command() {
        auto nPos = m_oTokens.pos();
        depth_guard oDepthGuard(this, nPos);
        // Parse block
        auto pBlock = parse_block(
            shell_token_type::TK_CLOSE_PARENTHESIS
        );
        if (pBlock == nullptr) {
            pBlock = std::make_unique<shell_node_null_command>(nPos);
        }
        // Build node
        return std::make_unique<shell_node_dollar_command>(
            nPos, std::move(pBlock)
        );
    }

    evaluable_ptr shell_parser::parse_command(
        const parse_mode nMode
    ) {
        // Fetch next
        auto pContent = parse_command_expression(nMode);
        if (pContent == nullptr)
            return std::make_unique<shell_node_null_command>(this->m_oIstream.size());
        return std::make_unique<shell_node_command>(
            std::move(pContent)
        );
    }


    std::unique_ptr<shell_node_command_expression>
    shell_parser::parse_command_expression(
        const parse_mode nMode
    ) {
        std::vector<expandable_ptr> vTokens;

        auto pToken = m_oTokens.get();
        bool bFoundDelimiter = false;


        while (pToken != nullptr && !bFoundDelimiter) {
            switch (pToken->m_nType) {
                case shell_token_type::TK_WORD: {
                    vTokens.push_back(parse_word(m_oTokens));
                    break;
                }
                case shell_token_type::TK_ESCAPED:
                case shell_token_type::TK_UNICODE: {
                    vTokens.push_back(parse_unicode());
                    break;
                }
                case shell_token_type::TK_SPACE: {
                    if (!vTokens.empty() && vTokens.back() != nullptr) {
                        vTokens.emplace_back(nullptr);
                    }
                    break;
                }

                case shell_token_type::TK_QUOTE_SIMPLE: {
                    vTokens.push_back(parse_quote_simple());
                    break;
                }
                case shell_token_type::TK_QUOTE_DOUBLE: {
                    vTokens.push_back(parse_quote_double());
                    break;
                }
                case shell_token_type::TK_QUOTE_BACK: {
                    if (has(nMode, parse_mode::PM_BACKQUOTE)) {
                        m_oTokens.put_back();
                        bFoundDelimiter = true;
                    } else {
                        vTokens.push_back(parse_quote_back());
                    }
                    break;
                }

                case shell_token_type::TK_DOLLAR: {
                    vTokens.push_back(parse_dollar());
                    break;
                }

                case shell_token_type::TK_CMD_SEPARATOR:
                case shell_token_type::TK_CLOSE_PARENTHESIS:
                case shell_token_type::TK_CLOSE_BRACKETS:
                case shell_token_type::TK_CLOSE_SQR_BRACKETS:
                case shell_token_type::TK_PIPE:
                case shell_token_type::TK_OR:
                case shell_token_type::TK_BACKGROUND:
                case shell_token_type::TK_AND: {
                    // Finish
                    m_oTokens.put_back();
                    bFoundDelimiter = true;
                    break;
                }

                case shell_token_type::TK_OPEN_BRACKETS: {
                    if (has(nMode, parse_mode::PM_FUNCTION_NAME)) {
                        // Finish
                        m_oTokens.put_back();
                        bFoundDelimiter = true;
                        break;
                    }
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
                }

                default:
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
            }

            if (!bFoundDelimiter)
                pToken = m_oTokens.get();
        }

        // Check there are tokens
        if (vTokens.empty()) {
            return nullptr;
        }

        // Construct command and return
        return std::make_unique<shell_node_command_expression>(
            std::move(vTokens)
        );
    }

    evaluable_ptr shell_parser::parse_command_group(
        const parse_mode nMode
    ) {
        // parse_command
        std::vector<evaluable_ptr> vExpressions;
        bool bFoundDelimiter = false;

        // First token
        const shell_token *pToken = m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))
            pToken = m_oTokens.get();
        auto nStartPos = m_oTokens.pos();

        while (pToken != nullptr && !bFoundDelimiter) {
            switch (pToken->m_nType) {
                case shell_token_type::TK_WORD: {
                    if (const auto nKeyword = m_oTokens.keyword();
                        nKeyword == shell_keyword::SK_NONE
                    ) {
                        // Parse command
                        m_oTokens.put_back();
                        vExpressions.push_back(parse_command(nMode));
                    } else {
                        vExpressions.push_back(parse_keyword(nKeyword, nMode));
                    }
                    break;
                }

                case shell_token_type::TK_ESCAPED:
                case shell_token_type::TK_UNICODE:
                case shell_token_type::TK_DOLLAR:
                case shell_token_type::TK_QUOTE_SIMPLE:
                case shell_token_type::TK_QUOTE_DOUBLE: {
                    // Parse command
                    m_oTokens.put_back();
                    auto pCommand = parse_command(nMode);
                    if (pCommand->get_type() != shell_node_type::SNT_NULL_COMMAND)
                        vExpressions.push_back(std::move(pCommand));
                    break;
                }

                case shell_token_type::TK_QUOTE_BACK: {
                    if (has(nMode, parse_mode::PM_BACKQUOTE)) {
                        m_oTokens.put_back();
                        bFoundDelimiter = true;
                    } else {
                        auto pCommand = parse_command(parse_mode::PM_BACKQUOTE);
                        if (pCommand->get_type() != shell_node_type::SNT_NULL_COMMAND)
                            vExpressions.push_back(std::move(pCommand));
                    }
                    break;
                }

                case shell_token_type::TK_OPEN_PARENTHESIS: {
                    // Parse sub content
                    vExpressions.push_back(parse_parentheses());
                    break;
                }
                case shell_token_type::TK_OPEN_BRACKETS: {
                    // Parse sub content
                    vExpressions.push_back(parse_brackets());
                    break;
                }
                case shell_token_type::TK_OPEN_SQR_BRACKETS: {
                    // Parse sub content
                    vExpressions.push_back(parse_sqr_brackets());
                    break;
                }

                case shell_token_type::TK_CMD_SEPARATOR:
                case shell_token_type::TK_CLOSE_PARENTHESIS:
                case shell_token_type::TK_CLOSE_BRACKETS:
                case shell_token_type::TK_CLOSE_SQR_BRACKETS: {
                    m_oTokens.put_back();
                    bFoundDelimiter = true;
                    break;
                }

                case shell_token_type::TK_BACKGROUND: {
                    if (vExpressions.empty()) {
                        throw shell_parser_exception{
                            shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                            m_oIstream.str(), pToken->m_nPos
                        };
                    }
                    auto pBack = std::move(vExpressions.back());
                    vExpressions.back() = std::make_unique<shell_node_background>(
                        pToken->m_nPos, std::move(pBack)
                    );
                    bFoundDelimiter = true;
                    break;
                }

                case shell_token_type::TK_PIPE:
                    parse_command_group_oper(
                        vExpressions,
                        shell_node_type::SNT_PIPE,
                        pToken->m_nPos, nMode
                    );
                    break;

                case shell_token_type::TK_AND:
                    parse_command_group_oper(
                        vExpressions,
                        shell_node_type::SNT_AND,
                        pToken->m_nPos, nMode
                    );
                    break;

                case shell_token_type::TK_OR:
                    parse_command_group_oper(
                        vExpressions,
                        shell_node_type::SNT_OR,
                        pToken->m_nPos, nMode
                    );
                    break;

                default:
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
            }

            if (!bFoundDelimiter) {
                pToken = m_oTokens.get();
                while (m_oTokens.is(shell_token_type::TK_SPACE))
                    pToken = m_oTokens.get();
            }
        }

        // Check there are expressions
        if (vExpressions.empty()) {
            return nullptr;
        }

        return std::make_unique<shell_node_command_block>(
            nStartPos, std::move(vExpressions)
        );
    }

    void shell_parser::parse_command_group_oper(
        std::vector<evaluable_ptr> &vExpressions,
        const shell_node_type nNodeType,
        const std::size_t nPos,
        const parse_mode nMode
    ) {
        // No left side
        if (vExpressions.empty()) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), nPos
            };
        }
        // Fetch right expression
        auto pExpression = parse_command_group(nMode);
        // No right side
        if (pExpression == nullptr) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), nPos
            };
        }
        // Make operator
        auto pOperator = shell_node_operator::make(
            nNodeType,
            nPos,
            std::move(vExpressions.back()),
            std::move(pExpression)
        );
        // Append operator
        vExpressions.back() = std::move(pOperator);
    }


    evaluable_ptr shell_parser::parse_parentheses() {
        auto nPos = m_oTokens.pos();
        // Increase depth
        depth_guard oDepthGuard(this, nPos);
        // Parse block
        auto pBlock = parse_block(
            shell_token_type::TK_CLOSE_PARENTHESIS
        );
        // Make sure executes something
        if (pBlock == nullptr)
            return std::make_unique<shell_node_null_command>(nPos);
        // Generate node
        std::vector<evaluable_ptr> vSubCommands;
        vSubCommands.push_back(std::move(pBlock));
        return std::make_unique<shell_node_command_block_subshell>(
            nPos, std::move(vSubCommands)
        );
    }

    evaluable_ptr shell_parser::parse_brackets() {
        auto nPos = m_oTokens.pos();
        // Increase depth
        depth_guard oDepthGuard(this, nPos);
        // Parse block
        auto pBlock = parse_block(
            shell_token_type::TK_CLOSE_BRACKETS
        );
        // Make sure executes something
        if (pBlock == nullptr)
            return std::make_unique<shell_node_null_command>(nPos);
        // Generate node
        std::vector<evaluable_ptr> vSubCommands;
        vSubCommands.push_back(std::move(pBlock));
        return std::make_unique<shell_node_command_block>(
            nPos, std::move(vSubCommands)
        );
    }

    evaluable_ptr shell_parser::parse_sqr_brackets() {
        auto nPos = m_oTokens.pos();
        auto pExpression = parse_test_expression();
        m_oTokens.get();
        if (!m_oTokens.is(shell_token_type::TK_CLOSE_SQR_BRACKETS)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SQR_BRACKETS,
                m_oIstream.str(), nPos
            };
        }
        if (pExpression == nullptr) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), nPos
            };
        }
        return std::make_unique<shell_node_test>(
            nPos, std::move(pExpression)
        );
    }

    evaluable_ptr shell_parser::parse_block(const shell_token_type nEnd) {
        // parse_command
        auto nStartPos = m_oTokens.pos();
        auto pToken = m_oTokens.get();
        std::vector<evaluable_ptr> vExpressions;

        while (pToken != nullptr && pToken->m_nType != nEnd) {
            switch (pToken->m_nType) {
                case shell_token_type::TK_WORD: {
                    if (
                        const auto nKeyword = m_oTokens.keyword();
                        nKeyword == shell_keyword::SK_NONE
                    ) {
                        // Parse command
                        m_oTokens.put_back();
                        const auto nMode = nEnd == shell_token_type::TK_QUOTE_BACK
                                               ? parse_mode::PM_BACKQUOTE
                                               : parse_mode::PM_NORMAL;
                        vExpressions.push_back(parse_command_group(nMode));
                    } else {
                        vExpressions.push_back(parse_keyword(nKeyword, parse_mode::PM_NORMAL));
                    }
                    break;
                }

                case shell_token_type::TK_OPEN_PARENTHESIS:
                case shell_token_type::TK_OPEN_BRACKETS:
                case shell_token_type::TK_OPEN_SQR_BRACKETS:
                case shell_token_type::TK_ESCAPED:
                case shell_token_type::TK_UNICODE:
                case shell_token_type::TK_DOLLAR:
                case shell_token_type::TK_QUOTE_SIMPLE:
                case shell_token_type::TK_QUOTE_DOUBLE: {
                    // Parse command
                    m_oTokens.put_back();
                    const auto nMode = nEnd == shell_token_type::TK_QUOTE_BACK
                                           ? parse_mode::PM_BACKQUOTE
                                           : parse_mode::PM_NORMAL;
                    vExpressions.push_back(parse_command_group(nMode));
                    break;
                }

                case shell_token_type::TK_QUOTE_BACK: {
                    // Parse command
                    vExpressions.push_back(parse_command_group(parse_mode::PM_BACKQUOTE));
                    break;
                }

                case shell_token_type::TK_SPACE:
                case shell_token_type::TK_CMD_SEPARATOR: {
                    break;
                }

                case shell_token_type::TK_CLOSE_PARENTHESIS:
                case shell_token_type::TK_CLOSE_BRACKETS:
                case shell_token_type::TK_CLOSE_SQR_BRACKETS: {
                    // Should not find any kind of closing tokens inside aside the end one
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
                }

                default:
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
            }

            pToken = m_oTokens.get();
        }

        if (
            (pToken != nullptr && pToken->m_nType != nEnd)
            || (pToken == nullptr && nEnd != shell_token_type::TK_EOF)
        ) {
            auto nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR;
            switch (nEnd) {
                case shell_token_type::TK_CLOSE_PARENTHESIS:
                    nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_PARENTHESES;
                    break;
                case shell_token_type::TK_CLOSE_BRACKETS:
                    nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_BRACKETS;
                    break;
                case shell_token_type::TK_CLOSE_SQR_BRACKETS:
                    nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SQR_BRACKETS;
                    break;
                case shell_token_type::TK_QUOTE_BACK:
                    nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_BACK_QUOTES;
                    break;
                default:
                    break; // nStatus remains SHELL_ERROR_SYNTAX_ERROR
            }
            throw shell_parser_exception{
                nStatus, m_oIstream.str(), nStartPos
            };
        }

        if (vExpressions.empty()) {
            return nullptr;
        }

        if (vExpressions.size() == 1) {
            return std::move(vExpressions.front());
        }

        return std::make_unique<shell_node_command_block>(
            nStartPos, std::move(vExpressions)
        );
    }

    evaluable_ptr shell_parser::parse_block(
        const shell_keyword nEnd,
        const parse_mode nMode
    ) {
        // parse_command
        auto nStartPos = m_oTokens.pos();
        auto pToken = m_oTokens.get();
        std::vector<evaluable_ptr> vExpressions;

        while (
            pToken != nullptr
            && !has(m_oTokens.keyword(), nEnd)
        ) {
            switch (pToken->m_nType) {
                case shell_token_type::TK_WORD: {
                    if (
                        const auto nKeyword = m_oTokens.keyword();
                        nKeyword == shell_keyword::SK_NONE
                    ) {
                        // Parse command
                        m_oTokens.put_back();
                        vExpressions.push_back(parse_command_group(nMode));
                    } else {
                        vExpressions.push_back(parse_keyword(nKeyword, nMode));
                    }
                    break;
                }

                case shell_token_type::TK_OPEN_PARENTHESIS:
                case shell_token_type::TK_OPEN_BRACKETS:
                case shell_token_type::TK_OPEN_SQR_BRACKETS:
                case shell_token_type::TK_ESCAPED:
                case shell_token_type::TK_UNICODE:
                case shell_token_type::TK_DOLLAR:
                case shell_token_type::TK_QUOTE_SIMPLE:
                case shell_token_type::TK_QUOTE_DOUBLE: {
                    // Parse command
                    m_oTokens.put_back();
                    auto nMode = pToken->m_nType == shell_token_type::TK_QUOTE_BACK
                                     ? parse_mode::PM_BACKQUOTE
                                     : parse_mode::PM_NORMAL;
                    if (has(nMode, parse_mode::PM_LOOP))
                        nMode &= parse_mode::PM_LOOP;
                    auto pGroup = parse_command_group(nMode);
                    vExpressions.push_back(std::move(pGroup));
                    break;
                }

                case shell_token_type::TK_QUOTE_BACK: {
                    // Parse command
                    auto pGroup = parse_command_group(parse_mode::PM_BACKQUOTE);
                    vExpressions.push_back(std::move(pGroup));
                    break;
                }

                case shell_token_type::TK_SPACE:
                case shell_token_type::TK_CMD_SEPARATOR: {
                    break;
                }

                case shell_token_type::TK_CLOSE_PARENTHESIS:
                case shell_token_type::TK_CLOSE_BRACKETS:
                case shell_token_type::TK_CLOSE_SQR_BRACKETS: {
                    // Should not find any kind of closing tokens inside aside the end one
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
                }

                default:
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
            }

            pToken = m_oTokens.get();
        }

        if (
            (pToken != nullptr && !has(m_oTokens.keyword(), nEnd))
            || (pToken == nullptr && nEnd != shell_keyword::SK_NONE)
        ) {
            auto nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR;
            switch (nEnd) {
                case shell_keyword::SK_IF_DELIMITER:
                    nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_IF;
                    break;
                case shell_keyword::SK_DONE:
                    nStatus = shell_status::SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_LOOP;
                    break;
                default:
                    break; // nStatus remains SHELL_ERROR_SYNTAX_ERROR
            }
            throw shell_parser_exception{
                nStatus, m_oIstream.str(), nStartPos
            };
        }

        if (vExpressions.empty()) {
            return nullptr;
        }

        if (vExpressions.size() == 1) {
            return std::move(vExpressions.front());
        }

        return std::make_unique<shell_node_command_block>(
            nStartPos, std::move(vExpressions)
        );
    }

    evaluable_ptr shell_parser::parse_keyword(
        const shell_keyword nKeyword,
        const parse_mode nMode
    ) {
        // Increase depth
        const auto nPos = m_oTokens.pos();
        depth_guard oDepthGuard(this, nPos);

        // Process keyword
        switch (nKeyword) {
            case shell_keyword::SK_IF:
                return parse_if();
            case shell_keyword::SK_FOR:
                return parse_for();
            case shell_keyword::SK_WHILE:
                return parse_while();
            case shell_keyword::SK_UNTIL:
                return parse_until();
            case shell_keyword::SK_CONTINUE: {
                if (has(nMode, parse_mode::PM_LOOP)) {
                    // Skip space
                    while (m_oTokens.is_next(shell_token_type::TK_SPACE))m_oTokens.get();
                    // Check cmd separator
                    if (
                        m_oTokens.next() == nullptr
                        || m_oTokens.is_next(shell_token_type::TK_CMD_SEPARATOR)
                        || m_oTokens.is_next(shell_token_type::TK_OR)
                        || m_oTokens.is_next(shell_token_type::TK_AND)
                    ) {
                        return std::make_unique<shell_node_continue>(nPos);
                    }
                }
                throw shell_parser_exception{
                    shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                    m_oIstream.str(), nPos
                };
            }
            case shell_keyword::SK_BREAK: {
                if (has(nMode, parse_mode::PM_LOOP)) {
                    // Skip space
                    while (m_oTokens.is_next(shell_token_type::TK_SPACE))m_oTokens.get();
                    // Check cmd separator
                    if (
                        m_oTokens.next() == nullptr
                        || m_oTokens.is_next(shell_token_type::TK_CMD_SEPARATOR)
                        || m_oTokens.is_next(shell_token_type::TK_OR)
                        || m_oTokens.is_next(shell_token_type::TK_AND)
                    ) {
                        return std::make_unique<shell_node_break>(nPos);
                    }
                }
                throw shell_parser_exception{
                    shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                    m_oIstream.str(), nPos
                };
            }

            case shell_keyword::SK_FUNCTION: {
                return parse_function();
            }
            default: {
                throw shell_parser_exception{
                    shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                    m_oIstream.str(), nPos
                };
            }
        }
    }

    expandable_ptr
    shell_parser::parse_test_expression(
    ) {
        std::vector<expandable_ptr> vTokens;

        auto pToken = m_oTokens.get();
        bool bFoundDelimiter = false;

        while (
            pToken != nullptr
            && pToken->m_nType != shell_token_type::TK_CLOSE_SQR_BRACKETS
        ) {
            switch (pToken->m_nType) {
                case shell_token_type::TK_WORD: {
                    vTokens.push_back(parse_word(m_oTokens));
                    break;
                }
                case shell_token_type::TK_ESCAPED:
                case shell_token_type::TK_UNICODE: {
                    vTokens.push_back(parse_unicode());
                    break;
                }
                case shell_token_type::TK_SPACE: {
                    if (!vTokens.empty() && vTokens.back() != nullptr) {
                        vTokens.emplace_back(nullptr);
                    }
                    break;
                }

                case shell_token_type::TK_QUOTE_SIMPLE: {
                    vTokens.push_back(parse_quote_simple());
                    break;
                }
                case shell_token_type::TK_QUOTE_DOUBLE: {
                    vTokens.push_back(parse_quote_double());
                    break;
                }
                case shell_token_type::TK_QUOTE_BACK: {
                    vTokens.push_back(parse_quote_back());
                    break;
                }

                case shell_token_type::TK_DOLLAR: {
                    vTokens.push_back(parse_dollar());
                    break;
                }

                case shell_token_type::TK_OR:
                case shell_token_type::TK_AND:
                case shell_token_type::TK_OPEN_PARENTHESIS:
                case shell_token_type::TK_CLOSE_PARENTHESIS: {
                    vTokens.push_back(parse_word(m_oTokens));
                    break;
                }

                case shell_token_type::TK_CLOSE_SQR_BRACKETS: {
                    // Finish
                    m_oTokens.put_back();
                    bFoundDelimiter = true;
                    break;
                }

                default:
                    throw shell_parser_exception{
                        shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                        m_oIstream.str(), pToken->m_nPos
                    };
            }

            if (!bFoundDelimiter)
                pToken = m_oTokens.get();
        }

        if (
            m_oTokens.is(shell_token_type::TK_CLOSE_SQR_BRACKETS)
        ) {
            // Put bach square brackets
            m_oTokens.put_back();
        }

        // Check there are tokens
        if (vTokens.empty()) {
            return nullptr;
        }

        // Construct command and return
        return std::make_unique<shell_node_command_expression>(
            std::move(vTokens)
        );
    }

    evaluable_ptr shell_parser::parse_if() {
        // Record if position
        const auto nPos = m_oTokens.pos();

        // Fetch if statement
        auto pCondition = parse_command_group(parse_mode::PM_NORMAL);

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        // Check semicolon
        if (!m_oTokens.is(shell_token_type::TK_CMD_SEPARATOR)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), m_oTokens.pos()
            };
        }

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        // Get "then" keyword
        if (!m_oTokens.keyword(shell_keyword::SK_THEN)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_THEN,
                m_oIstream.str(), nPos
            };
        }

        // Parse block
        auto pBlock = parse_block(shell_keyword::SK_IF_DELIMITER, parse_mode::PM_NORMAL);
        if (pBlock == nullptr)pBlock = std::make_unique<shell_node_null_command>(m_oTokens.pos());

        // Case else
        if (m_oTokens.keyword(shell_keyword::SK_ELSE)) {
            depth_guard oDepthGuard(this, nPos);
            auto pElse = parse_block(shell_keyword::SK_IF_DELIMITER, parse_mode::PM_NORMAL);
            return std::make_unique<shell_node_if>(
                nPos,
                std::move(pCondition),
                std::move(pBlock),
                std::move(pElse)
            );
        }
        if (m_oTokens.keyword(shell_keyword::SK_ELIF)) {
            depth_guard oDepthGuard(this, nPos);
            auto pElse = parse_if();
            return std::make_unique<shell_node_if>(
                nPos,
                std::move(pCondition),
                std::move(pBlock),
                std::move(pElse)
            );
        }
        if (m_oTokens.keyword(shell_keyword::SK_FI)) {
            return std::make_unique<shell_node_if>(
                nPos,
                std::move(pCondition),
                std::move(pBlock),
                nullptr
            );
        }
        throw shell_parser_exception{
            shell_status::SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_IF,
            m_oIstream.str(), nPos
        };
    }

    evaluable_ptr shell_parser::parse_for() {
        // Record if position
        const auto nPos = m_oTokens.pos();

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        if (
            !m_oTokens.is(shell_token_type::TK_WORD)
            || !is_var(m_oTokens.current()->m_sTokenText)
        ) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_VARIABLE_NAME,
                m_oIstream.str(), nPos
            };
        }
        std::string sVariable(m_oTokens.current()->m_sTokenText);

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        // Get "in" keyword
        if (!m_oTokens.keyword(shell_keyword::SK_IN)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_IN,
                m_oIstream.str(), nPos
            };
        }

        // Get sequence
        auto pSequence = parse_command_expression(parse_mode::PM_NORMAL);

        m_oTokens.get(); // Reget separator

        // Check semicolon
        if (!m_oTokens.is(shell_token_type::TK_CMD_SEPARATOR)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), m_oTokens.pos()
            };
        }

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        // Get "do" keyword
        if (!m_oTokens.keyword(shell_keyword::SK_DO)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_DO,
                m_oIstream.str(), nPos
            };
        }

        // Get block
        depth_guard oDepthGuard(this, nPos);
        auto pIterative = parse_block(shell_keyword::SK_DONE, parse_mode::PM_LOOP);
        if (pIterative == nullptr)
            pIterative = std::make_unique<shell_node_null_command>(m_oTokens.pos());

        return std::make_unique<shell_node_for>(
            nPos, std::move(sVariable),
            std::move(pSequence),
            std::move(pIterative)
        );
    }

    evaluable_ptr shell_parser::parse_while() {
        // Record if position
        const auto nPos = m_oTokens.pos();

        // Get condition
        auto pCondition = parse_command_group(parse_mode::PM_NORMAL);

        // Check semicolon
        m_oTokens.get();
        if (!m_oTokens.is(shell_token_type::TK_CMD_SEPARATOR)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), m_oTokens.pos()
            };
        }

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        // Get "do" keyword
        if (!m_oTokens.keyword(shell_keyword::SK_DO)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_DO,
                m_oIstream.str(), nPos
            };
        }

        // Get block
        depth_guard oDepthGuard(this, nPos);
        auto pIterative = parse_block(shell_keyword::SK_DONE, parse_mode::PM_LOOP);
        if (pIterative == nullptr)
            pIterative = std::make_unique<shell_node_null_command>(m_oTokens.pos());

        return std::make_unique<shell_node_while>(
            nPos,
            std::move(pCondition),
            std::move(pIterative)
        );
    }

    evaluable_ptr shell_parser::parse_until() {
        // Record if position
        const auto nPos = m_oTokens.pos();

        // Get condition
        auto pCondition = parse_command_group(parse_mode::PM_NORMAL);

        // Check semicolon
        m_oTokens.get();
        if (!m_oTokens.is(shell_token_type::TK_CMD_SEPARATOR)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN,
                m_oIstream.str(), m_oTokens.pos()
            };
        }

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        // Get "do" keyword
        if (!m_oTokens.keyword(shell_keyword::SK_DO)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_DO,
                m_oIstream.str(), nPos
            };
        }

        // Get block
        depth_guard oDepthGuard(this, nPos);
        auto pIterative = parse_block(shell_keyword::SK_DONE, parse_mode::PM_LOOP);
        if (pIterative == nullptr)
            pIterative = std::make_unique<shell_node_null_command>(m_oTokens.pos());

        return std::make_unique<shell_node_until>(
            nPos,
            std::move(pCondition),
            std::move(pIterative)
        );
    }

    evaluable_ptr shell_parser::parse_function() {
        // Position
        const auto nPos = m_oTokens.pos();

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        // Function name
        m_oTokens.put_back();
        auto pName = parse_command_expression(parse_mode::PM_FUNCTION_NAME);
        if (pName == nullptr) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_FUNCTION_NAME,
                this->m_oIstream.str(), nPos
            };
        }

        // Skip empty spaces
        m_oTokens.get();
        while (m_oTokens.is(shell_token_type::TK_SPACE))m_oTokens.get();

        // Brackets
        if (!m_oTokens.is(shell_token_type::TK_OPEN_BRACKETS)) {
            throw shell_parser_exception{
                shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_FUNCTION_BODY,
                m_oIstream.str(), nPos
            };
        }

        // Function block
        auto nBlockPos = m_oTokens.pos();
        auto pBody = parse_block(shell_token_type::TK_CLOSE_BRACKETS);
        if (pBody == nullptr) {
            pBody = std::make_unique<shell_node_null_command>(nBlockPos);
        }
        return std::make_unique<shell_node_function>(
            nPos,
            std::move(pName),
            std::move(pBody)
        );
    }
}
