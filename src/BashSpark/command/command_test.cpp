/**
 * @file command_test.cpp
 * @brief Implements command `bs::command_test`.
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

#include "BashSpark/command/command_test.h"

#include <map>
#include <regex>

#include "BashSpark/tools/shell_def.h"

using namespace std::string_view_literals;

namespace bs {

    namespace {
        bool str_empty(const std::string_view &s) {
            if (s.empty()) return true;
            for (const char c: s) {
                if (c != ' ' && c != '\t' && c != '\n') return false;
            }
            return false;
        }
    }

    /**
     * @brief Exception representing a test-related error during expression evaluation.
     */
    struct test_error final : std::exception {
        /**
         * @brief Construct a new test_error.
         * @param nStatus Status code describing the type of math error.
         */
        explicit test_error(const shell_status nStatus)
            : m_nStatus(nStatus) {
        }

        /// Status/error code for the failure
        shell_status m_nStatus;
    };

    /**
     * @enum test_operator
     * @brief An enumeration representing various comparison and logical operators.
     *
     * This enum class defines a set of operators that can be used in expressions
     * to perform comparisons, checks, or logical operations.
     */
    enum class test_operator {
        TO_NONE ///< Represents no operation.
        , TO_EOF ///< Represents the end of a file condition.
        , TO_EQUALS ///< Represents equality comparison.
        , TO_MATCHES ///< Represents matching condition.
        , TO_GREATER_THAN ///< Represents greater than comparison.
        , TO_LESS_THAN ///< Represents less than comparison.
        , TO_GREATER_THAN_OR_EQUALS ///< Represents greater than or equal to comparison.
        , TO_LESS_THAN_OR_EQUALS ///< Represents less than or equal to comparison.
        , TO_NOT_EQUALS ///< Represents not equal comparison.
        , TO_EMPTY ///< Represents an empty condition.
        , TO_NON_EMPTY ///< Represents a non-empty condition.
        , TO_OR ///< Represents logical OR operation.
        , TO_AND ///< Represents logical AND operation.
        , TO_PARENTHESIS_OPEN ///< Represents an opening parenthesis.
        , TO_PARENTHESIS_CLOSE ///< Represents a closing parenthesis.
    };

    /**
     * @struct test_parser
     * @brief Structure to help class `command_test` parse it's parameters.
     */
    struct test_parser {
        /// Maximum depth of the shell parser
        static constexpr std::size_t MAX_DEPTH = 512;

        /**
         * @struct depth_guard
         * @brief Manages depth of a test_parser instance during its lifetime.
         *
         * Automatically increases depth upon creation and decreases it upon destruction.
         */
        struct depth_guard {
        public:
            /**
             * @brief Increases the parser's depth.
             * @param pParser Pointer to the test_parser instance.
             */
            explicit depth_guard(test_parser *pParser)
                : m_pParser(pParser) {
                m_pParser->increase_depth();
            }

            /**
             * @brief Decreases the parser's depth upon destruction.
             */
            ~depth_guard() {
                m_pParser->decrease_depth();
            }

        private:
            /// Pointer to the managed test_parser.
            test_parser *m_pParser;
        };

        /// Map containing operator expressions and codes
        static std::map<std::string_view, test_operator> OPERATORS;

        /**
         * Contstructs a test_parser
         * @param vTokens Tokens to parse
         */
        explicit test_parser(
            const std::span<const std::string> &vTokens
        ) noexcept
            : m_vTokens(vTokens) {
        }

        /**
        * @brief Evaluate a full expression (top-level entry point).
        * @param nPos Token positon
        * @return Operation result
        */
        bool do_toplevel(std::size_t &nPos);

    private:
        /**
        * @brief Returns the operator at the given token position.
        *
        * @param nPos Index of the token.
        * @return test_operator The resolved operator, or MO_EOF/MO_NONE.
        */
        [[nodiscard]] test_operator get_operator(
            const std::size_t nPos
        ) const noexcept {
            if (nPos >= this->m_vTokens.size()) return test_operator::TO_EOF;
            const auto pIter = OPERATORS.find(this->m_vTokens[nPos]);
            return pIter != OPERATORS.end() ? pIter->second : test_operator::TO_NONE;
        }

        /**
         * @brief Parse and evaluate a or expression.
         * @param nPos Token positon
         * @return Operation result
         */
        bool do_or(std::size_t &nPos);

        /**
         * @brief Parse and evaluate an and expression.
         * @param nPos Token positon
         * @return Operation result
        */
        bool do_and(std::size_t &nPos);

        /**
         * @brief Parse and evaluate a primary expression: unary/binary operators, etc.
         * @param nPos Token positon
         * @return Operation result
         */
        bool do_expr(std::size_t &nPos);

        /**
         * @brief Parse (...) parentheses content.
         * @param nPos Token positon
         * @return Operation result
         */
        bool do_parentheses(std::size_t &nPos);

    public:
        /**
         * @brief Increases the current recursion/evaluation depth.
         * Ensures that the depth does not exceed MAX_DEPTH.
         * @throws math_error If increasing the depth would exceed MAX_DEPTH.
         */
        void increase_depth() {
            if (this->m_nDepth + 1 > MAX_DEPTH)
                throw test_error(shell_status::SHELL_CMD_ERROR_MATH_MAX_DEPTH_REACHED);
            ++this->m_nDepth;
        }

        /**
         * @brief Decreases the current recursion/evaluation depth.
         * Will not decrement below zero.
         */
        void decrease_depth() {
            if (this->m_nDepth > 0)
                --this->m_nDepth;
        }

    private:
        const std::span<const std::string> &m_vTokens;
        std::size_t m_nDepth = 0;
    };

    std::map<std::string_view, test_operator> test_parser::OPERATORS = {
        {"-eq", test_operator::TO_EQUALS},
        {"==", test_operator::TO_EQUALS},
        {"-ne", test_operator::TO_NOT_EQUALS},
        {"!=", test_operator::TO_NOT_EQUALS},
        {"=~", test_operator::TO_MATCHES},
        {"-gt", test_operator::TO_GREATER_THAN},
        {">", test_operator::TO_GREATER_THAN},
        {"-lt", test_operator::TO_LESS_THAN},
        {"<", test_operator::TO_LESS_THAN},
        {"-ge", test_operator::TO_GREATER_THAN_OR_EQUALS},
        {">=", test_operator::TO_GREATER_THAN_OR_EQUALS},
        {"-le", test_operator::TO_LESS_THAN_OR_EQUALS},
        {"<=", test_operator::TO_LESS_THAN_OR_EQUALS},
        {"<=", test_operator::TO_EMPTY},
        {"-o", test_operator::TO_OR},
        {"||", test_operator::TO_OR},
        {"-a", test_operator::TO_AND},
        {"&&", test_operator::TO_AND},
        {"(", test_operator::TO_PARENTHESIS_OPEN},
        {")", test_operator::TO_PARENTHESIS_CLOSE},
        {"-n", test_operator::TO_NON_EMPTY},
        {"-z", test_operator::TO_EMPTY},
    };

    shell_status command_test::run(
        const std::span<const std::string> &vArgs,
        shell_session &oSession
    ) const {
        if (vArgs.empty()) return shell_status::SHELL_SUCCESS;

        auto nStatus = shell_status::SHELL_SUCCESS;
        try {
            std::size_t nPos = 0;
            const auto pParser = std::make_unique<test_parser>(vArgs);
            const auto bResult = pParser->do_toplevel(nPos);
            return bResult ? shell_status::SHELL_SUCCESS : shell_status::SHELL_CMD_TEST_FALSE;
        } catch (test_error &e) {
            nStatus = e.m_nStatus;
            this->msg_error_test(oSession.err(), nStatus);
            return nStatus;
        }
    }

    void command_test::msg_error_test(std::ostream &oStdErr, const shell_status nStatus) const {
        switch (nStatus) {
            case shell_status::SHELL_CMD_ERROR_TEST_UNCLOSED_PARENTHESIS:
                oStdErr << "Error: Unclosed parenthesis in the command.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION:
                oStdErr << "Error: The expression provided is malformed.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_REGEX:
                oStdErr << "Error: The regular expression is malformed.\n";
                break;
            default:
                break;
        }
    }

    bool test_parser::do_toplevel(
        std::size_t &nPos
    ) {
        depth_guard oDepthGuard(this);
        return do_or(nPos);
    }

    bool test_parser::do_or(
        std::size_t &nPos
    ) {
        depth_guard oDepthGuard(this);

        auto nLeft = do_and(nPos);

        do {
            switch (get_operator(nPos)) {
                case test_operator::TO_OR: {
                    nPos += 1;
                    const bool nRight = do_and(nPos);
                    nLeft = nLeft || nRight;
                    break;
                }
                default:
                    return nLeft;
            }
        } while (true);
    }

    bool test_parser::do_and(
        std::size_t &nPos
    ) {
        depth_guard oDepthGuard(this);

        auto nLeft = do_expr(nPos);

        do {
            switch (get_operator(nPos)) {
                case test_operator::TO_AND: {
                    nPos += 1;
                    const bool nRight = do_expr(nPos);
                    nLeft = nLeft && nRight;
                    break;
                }
                default:
                    return nLeft;
            }
        } while (true);
    }

    bool test_parser::do_parentheses(
        std::size_t &nPos
    ) {
        depth_guard oDepthGuard(this);

        const auto nContent = do_toplevel(nPos);

        if (nPos >= m_vTokens.size() || m_vTokens[nPos] != ")"sv)
            throw test_error(shell_status::SHELL_CMD_ERROR_TEST_UNCLOSED_PARENTHESIS);

        nPos += 1;
        return nContent;
    }

    bool test_parser::do_expr(std::size_t &nPos) {
        if (nPos >= this->m_vTokens.size()) throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);

        if (get_operator(nPos) == test_operator::TO_PARENTHESIS_OPEN) {
            nPos += 1;
            return do_parentheses(nPos);
        }

        // Ensure first item + operator
        if (nPos + 1 >= this->m_vTokens.size())
            throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);

        switch (
            auto nOper = get_operator(nPos + 1)
        ) {
            case test_operator::TO_EQUALS: {
                if (nPos + 2 >= this->m_vTokens.size())
                    throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
                if (is_number(m_vTokens[nPos]) && is_number(m_vTokens[nPos + 2])) {
                    const auto n1 = std::stoull(m_vTokens[nPos + 0]);
                    const auto n2 = std::stoull(m_vTokens[nPos + 2]);
                    nPos += 3;
                    return n1 == n2;
                }
                const auto bResult = m_vTokens[nPos + 0] == m_vTokens[nPos + 2];
                nPos += 3;
                return bResult;
            }
            case test_operator::TO_MATCHES: {
                if (nPos + 2 >= this->m_vTokens.size())
                    throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
                try {
                    const std::regex pattern(m_vTokens[nPos + 2]);
                    const auto bResult = std::regex_match(m_vTokens[nPos + 0], pattern);
                    nPos += 3;
                    return bResult;
                } catch (const std::regex_error &) {
                    throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_REGEX);
                }
            }
            case test_operator::TO_GREATER_THAN: {
                if (nPos + 2 >= this->m_vTokens.size())
                    throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
                if (is_number(m_vTokens[nPos]) && is_number(m_vTokens[nPos + 2])) {
                    const auto n1 = std::stoull(m_vTokens[nPos + 0]);
                    const auto n2 = std::stoull(m_vTokens[nPos + 2]);
                    nPos += 3;
                    return n1 > n2;
                }
                const auto bResult = m_vTokens[nPos + 0] > m_vTokens[nPos + 2];
                nPos += 3;
                return bResult;
            }
            case test_operator::TO_LESS_THAN: {
                if (nPos + 2 >= this->m_vTokens.size())
                    throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
                if (is_number(m_vTokens[nPos]) && is_number(m_vTokens[nPos + 2])) {
                    const auto n1 = std::stoull(m_vTokens[nPos + 0]);
                    const auto n2 = std::stoull(m_vTokens[nPos + 2]);
                    nPos += 3;
                    return n1 < n2;
                }
                const auto bResult = m_vTokens[nPos + 0] < m_vTokens[nPos + 2];
                nPos += 3;
                return bResult;
            }
            case test_operator::TO_GREATER_THAN_OR_EQUALS: {
                if (nPos + 2 >= this->m_vTokens.size())
                    throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
                if (is_number(m_vTokens[nPos]) && is_number(m_vTokens[nPos + 2])) {
                    const auto n1 = std::stoull(m_vTokens[nPos + 0]);
                    const auto n2 = std::stoull(m_vTokens[nPos + 2]);
                    return n1 >= n2;
                }
                return m_vTokens[nPos + 0] >= m_vTokens[nPos + 2];
            }
            case test_operator::TO_LESS_THAN_OR_EQUALS: {
                if (nPos + 2 >= this->m_vTokens.size())
                    throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
                if (is_number(m_vTokens[nPos]) && is_number(m_vTokens[nPos + 2])) {
                    const auto n1 = std::stoull(m_vTokens[nPos + 0]);
                    const auto n2 = std::stoull(m_vTokens[nPos + 2]);
                    nPos += 3;
                    return n1 <= n2;
                }
                const auto bResult = m_vTokens[nPos + 0] <= m_vTokens[nPos + 2];
                nPos += 3;
                return bResult;
            }
            case test_operator::TO_NOT_EQUALS: {
                if (nPos + 2 >= this->m_vTokens.size())
                    throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
                if (is_number(m_vTokens[nPos]) && is_number(m_vTokens[nPos + 2])) {
                    const auto n1 = std::stoull(m_vTokens[nPos + 0]);
                    const auto n2 = std::stoull(m_vTokens[nPos + 2]);
                    nPos += 3;
                    return n1 != n2;
                }
                const auto bResult = m_vTokens[nPos + 0] != m_vTokens[nPos + 2];
                nPos += 3;
                return bResult;
            }
            case test_operator::TO_NONE: {
                nOper = get_operator(nPos);
                switch (nOper) {
                    case test_operator::TO_EMPTY: {
                        const auto bResult = str_empty(m_vTokens[nPos + 1]);
                        nPos += 2;
                        return bResult;
                    }
                    case test_operator::TO_NON_EMPTY: {
                        const auto bResult = !str_empty(m_vTokens[nPos + 1]);
                        nPos += 2;
                        return bResult;
                    }
                    default: {
                        throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
                    }
                }
            }
            default: {
                throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
            }
        }


        if (nPos + 2 >= this->m_vTokens.size()) {
            // Unary oper for sure
            auto nOper = get_operator(nPos + 1);
            if (nOper != test_operator::TO_NONE)
                throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
            else if (nPos + 1 >= this->m_vTokens.size()) {
                auto nOper = get_operator(nPos + 1);
            } else {
                throw test_error(shell_status::SHELL_CMD_ERROR_TEST_MALFORMED_EXPRESSION);
            }
        }
    }
}
