//
// Created by dante on 2/12/25.
//

#include "BashSpark/command/command_math.h"

#include <map>
#include <optional>

#include <boost/safe_numerics/safe_integer.hpp>

#include "BashSpark/tools/shell_def.h"

using namespace std::string_view_literals;

namespace bs {
    /// Sugar syntax: typedef of boost::safe_numerics::safe<std::int64_t>
    using safe_int = boost::safe_numerics::safe<std::int64_t>;

    /**
     * @brief Enumeration of all supported mathematical operators.
     */
    enum class math_operator {
        MO_NONE, ///< No operator
        MO_PLUS, ///< Addition
        MO_MINUS, ///< Subtraction
        MO_MULT, ///< Multiplication
        MO_DIV, ///< Division
        MO_MOD, ///< Modulo
        MO_POW, ///< Power
        MO_EOF, ///< End of formula/input
        MO_FACTORIAL, ///< Factorial
        MO_SIGN, ///< Unary sign (+/-)
        MO_ABS, ///< Absolute value
        MO_SUM, ///< Summation
        MO_PRODUCT, ///< Product
        MO_FUNC_ARG_SEP, ///< Function argument separator (e.g., comma)
        MO_OPEN_PARENTHESIS, ///< '(' token
        MO_CLOSE_PARENTHESIS ///< ')' token
    };

    /**
     * @brief Exception representing a math-related error during expression evaluation.
     */
    struct math_error final : std::exception {
        /**
         * @brief Construct a new math_error.
         * @param nStatus Status code describing the type of math error.
         */
        explicit math_error(const shell_status nStatus)
            : m_nStatus(nStatus) {
        }

        /// Status/error code for the failure
        shell_status m_nStatus;
    };

    namespace {
        /**
         * @brief Safely compute a + b using boost::safe_numerics.
         *
         * @param a Left operand.
         * @param b Right operand.
         * @return Sum of a and b.
         * @throws math_error On overflow or underflow.
         */
        [[nodiscard]] std::int64_t sum(const std::int64_t a, const std::int64_t b) {
            try {
                return safe_int(a) + safe_int(b);
            } catch (const boost::safe_numerics::safe_numerics_error &) {
                if (a > 0)
                    throw math_error(shell_status::SHELL_CMD_ERROR_MATH_OVERFLOW);
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_UNDERFLOW);
            }
        }

        /**
         * @brief Safely compute a - b using boost::safe_numerics.
         *
         * @param a Left operand.
         * @param b Right operand.
         * @return a - b result.
         * @throws math_error On overflow or underflow.
         */
        [[nodiscard]] std::int64_t rest(const std::int64_t a, const std::int64_t b) {
            try {
                return safe_int(a) - safe_int(b);
            } catch (const boost::safe_numerics::safe_numerics_error &) {
                if (a > 0)
                    throw math_error(shell_status::SHELL_CMD_ERROR_MATH_OVERFLOW);
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_UNDERFLOW);
            }
        }

        /**
         * @brief Safely compute a * b using boost::safe_numerics.
         *
         * @param a Left operand.
         * @param b Right operand.
         * @return Product of a and b.
         * @throws math_error On overflow or underflow.
         */
        [[nodiscard]] std::int64_t mult(const std::int64_t a, const std::int64_t b) {
            try {
                return safe_int(a) * safe_int(b);
            } catch (const boost::safe_numerics::safe_numerics_error &) {
                if (a > 0)
                    throw math_error(shell_status::SHELL_CMD_ERROR_MATH_OVERFLOW);
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_UNDERFLOW);
            }
        }

        /**
         * @brief Compute a / b with explicit division-by-zero protection.
         *
         * @param a Dividend.
         * @param b Divisor.
         * @return Quotient of the division.
         * @throws math_error If b == 0.
         */
        [[nodiscard]] std::int64_t div(const std::int64_t a, const std::int64_t b) {
            if (b == 0)
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_DIV_BY_ZERO);
            return a / b;
        }

        /**
         * @brief Compute a % b with explicit division-by-zero protection.
         *
         * @param a Dividend.
         * @param b Divisor.
         * @return Remainder of the division.
         * @throws math_error If b == 0.
         */
        [[nodiscard]] std::int64_t mod(const std::int64_t a, const std::int64_t b) {
            if (b == 0)
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_DIV_BY_ZERO);
            return a % b;
        }

        /**
         * @brief Compute a^b (a raised to power b) with overflow protection.
         *
         * Handles special cases such as 0^0, negative exponents, and trivial powers.
         *
         * @param a Base.
         * @param b Exponent.
         * @return a raised to the power b.
         * @throws math_error On overflow or invalid inputs (0^0).
         */
        [[nodiscard]] std::int64_t pow(const std::int64_t a, const std::int64_t b) {
            // Assert
            if (a == 0 && b == 0)
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_POW_0_EXP_0);

            // Base cases
            if (b < 0) return 0;
            if (b == 0) return 1;
            if (a == 0 || a == 1 || b == 1) return a;

            // Calculation
            safe_int nRes = 1;
            const safe_int sa = a;
            try {
                for (std::size_t i = 0; i < b; ++i)
                    nRes *= sa;
            } catch (const boost::safe_numerics::safe_numerics_error &) {
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_OVERFLOW);
            }
            return nRes;
        }

        /**
         * @brief Compute factorial(n) with overflow protection.
         *
         * @param n Input value (must be >= 0).
         * @return n! (factorial of n).
         * @throws math_error If n < 0 or result overflows.
         */
        [[nodiscard]] std::int64_t factorial(const std::int64_t n) {
            // Assert
            if (n < 0)
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_FACTORIAL_NEGATIVE);

            // Base cases
            if (n == 0 || n == 1) return 1;

            // Calculation
            safe_int nFactorial = 1;
            try {
                for (std::size_t i = 2; i <= n; ++i)
                    nFactorial *= safe_int(i);
            } catch (const boost::safe_numerics::safe_numerics_error &) {
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_OVERFLOW);
            }
            return nFactorial;
        }
    } // namespace

    /**
     * @struct math_parser
     * @brief Structure to help class `command_math` parse it's parameters.
     */
    struct math_parser {
        /// Maximum depth of the shell parser
        static constexpr std::size_t MAX_DEPTH = 512;

        /**
         * @struct expvar
         * @brief Structure to hold variable information in sequence functions
         */
        struct expvar {
            /// Map holding the pairs of variable, value
            std::unordered_map<std::string_view, std::int64_t, shell_hash_view> m_mVariables;

            /**
             * @brief Gets the value of a variable
             * @param sValue Value to get
             * @param oValue Variable to set the value (only set if found)
             * @return Whether the variable exits
             */
            bool get_value(const std::string &sValue, std::int64_t &oValue) const {
                const std::string_view sKey{sValue.data(), sValue.length()};
                const auto pIterator = m_mVariables.find(sKey);
                if (pIterator == m_mVariables.end()) return false;
                oValue = pIterator->second;
                return true;
            }

            /**
             * @brief Access a variable value, if not set default value is 0.
             * @param sValue Value to access
             * @return Reference to value of variable
             */
            std::int64_t &operator[](const std::string_view &sValue) {
                return this->m_mVariables[sValue];
            }
        };

        /**
         * @struct depth_guard
         * @brief Manages depth of a math_parser instance during its lifetime.
         *
         * Automatically increases depth upon creation and decreases it upon destruction.
         */
        struct depth_guard {
        public:
            /**
             * @brief Increases the parser's depth.
             * @param pParser Pointer to the math_parser instance.
             */
            explicit depth_guard(math_parser *pParser)
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
            /// Pointer to the managed math_parser.
            math_parser *m_pParser;
        };

        /// Map containing operator expressions and codes
        static std::map<std::string_view, math_operator> OPERATORS;

        /**
         * Contstructs a math_parser
         * @param vTokens Tokens to parse
         */
        explicit math_parser(
            const std::span<const std::string> &vTokens
        ) noexcept
            : m_vTokens(vTokens) {
        }


        /**
         * @brief Evaluate a full expression (top-level entry point).
         * @param nPos Token positon
         * @param nX Variable map
         * @return Operation result
         */
        std::int64_t do_toplevel(std::size_t &nPos, const expvar &nX);

    private:
        /**
        * @brief Returns the operator at the given token position.
        *
        * @param nPos Index of the token.
        * @return math_operator The resolved operator, or MO_EOF/MO_NONE.
        */
        [[nodiscard]] math_operator get_operator(
            const std::size_t nPos
        ) const noexcept {
            if (nPos >= this->m_vTokens.size()) return math_operator::MO_EOF;
            const auto pIter = OPERATORS.find(this->m_vTokens[nPos]);
            return pIter != OPERATORS.end() ? pIter->second : math_operator::MO_NONE;
        }

        /**
         * @brief Parse and evaluate a sum/subtraction expression.
         * @param nPos Token positon
         * @param nX Variable map
         * @return Operation result
         */
        std::int64_t do_sum(std::size_t &nPos, const expvar &nX);

        /**
         * @brief Parse and evaluate a multiplication/division/modulo expression.
         * @param nPos Token positon
         * @param nX Variable map
         * @return Operation result
        */
        std::int64_t do_mult(std::size_t &nPos, const expvar &nX);

        /**
         * @brief Parse and evaluate a power expression.
         * @param nPos Token positon
         * @param nX Variable map
         * @return Operation result
         */
        std::int64_t do_pow(std::size_t &nPos, const expvar &nX);

        /**
         * @brief Parse and evaluate a primary expression: value, variable, sign, parentheses, factorial, etc.
         * @param nPos Token positon
         * @param nX Variable map
         * @return Operation result
         */
        std::int64_t do_expr(std::size_t &nPos, const expvar &nX);

        /**
         * @brief Parse (...) parentheses content.
         * @param nPos Token positon
         * @param nX Variable map
         * @return Operation result
         */
        std::int64_t do_parentheses(std::size_t &nPos, const expvar &nX);

        /**
         * @brief Parse and compute SUM(var, start, step, end, expr).
         * @param nPos Token positon
         * @param nX Variable map
         * @return Operation result
         */
        std::int64_t do_list_sum(std::size_t &nPos, const expvar &nX);

        /**
         * @brief Parse and compute PRODUCT(var, start, step, end, expr).
         * @param nPos Token positon
         * @param nX Variable map
         * @return Operation result
         */
        std::int64_t do_list_mult(std::size_t &nPos, const expvar &nX);

    public:
        /**
         * @brief Increases the current recursion/evaluation depth.
         * Ensures that the depth does not exceed MAX_DEPTH.
         * @throws math_error If increasing the depth would exceed MAX_DEPTH.
         */
        void increase_depth() {
            if (this->m_nDepth + 1 > MAX_DEPTH)
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MAX_DEPTH_REACHED);
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

    shell_status command_math::run(
        const std::span<const std::string> &vArgs,
        shell_session &oSession
    ) const {
        auto nStatus = shell_status::SHELL_SUCCESS;
        try {
            std::size_t nPos = 0;
            const auto pParser = std::make_unique<math_parser>(vArgs);
            const math_parser::expvar nX;
            const auto nResult = pParser->do_toplevel(nPos, nX);
            oSession.out() << nResult;
        } catch (math_error &e) {
            nStatus = e.m_nStatus;
            this->msg_error_math(oSession.err(), nStatus);
        }
        return nStatus;
    }

    void command_math::msg_error_math(std::ostream &oStdErr, const shell_status nStatus) const {
        switch (nStatus) {
            case shell_status::SHELL_CMD_ERROR_MATH_NOT_AN_INTEGER:
                oStdErr << "math: value is not an integer.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_OVERFLOW:
                oStdErr << "math: arithmetic overflow.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_UNDERFLOW:
                oStdErr << "math: arithmetic underflow.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_DIV_BY_ZERO:
                oStdErr << "math: division by zero.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_POW_0_EXP_0:
                oStdErr << "math: undefined expression 0^0.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_FACTORIAL_NEGATIVE:
                oStdErr << "math: factorial of a negative number.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION:
                oStdErr << "math: malformed expression.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_MAX_DEPTH_REACHED:
                oStdErr << "math: expression nesting too deep.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_INVALID_VARIABLE_NAME:
                oStdErr << "math: invalid variable name.\n";
                break;
            case shell_status::SHELL_CMD_ERROR_MATH_SEQ_ITERATION_LOGIC:
                oStdErr << "math: invalid sequence iteration logic.\n";
                break;
            default:
                oStdErr << "math: unknown math error.\n";
                break;
        }
    }

    std::map<std::string_view, math_operator> math_parser::OPERATORS = {
        {"+", math_operator::MO_PLUS},
        {"-", math_operator::MO_MINUS},
        {"*", math_operator::MO_MULT},
        {"/", math_operator::MO_DIV},
        {"%", math_operator::MO_MOD},
        {"^", math_operator::MO_POW},
        {"**", math_operator::MO_POW},
        {"\u00D7", math_operator::MO_MULT},
        {"\u00F7", math_operator::MO_DIV},
        {"factorial", math_operator::MO_FACTORIAL},
        {"sign", math_operator::MO_SIGN},
        {"abs", math_operator::MO_ABS},
        {"sum", math_operator::MO_SUM},
        {"product", math_operator::MO_PRODUCT},
        {",", math_operator::MO_FUNC_ARG_SEP},
        {"(", math_operator::MO_OPEN_PARENTHESIS},
        {")", math_operator::MO_CLOSE_PARENTHESIS},
    };

    std::int64_t math_parser::do_toplevel(
        std::size_t &nPos,
        const expvar &nX
    ) {
        depth_guard oDepthGuard(this);
        return do_sum(nPos, nX);
    }

    std::int64_t math_parser::do_sum(
        std::size_t &nPos,
        const expvar &nX
    ) {
        depth_guard oDepthGuard(this);

        auto nLeft = do_mult(nPos, nX);

        do {
            switch (get_operator(nPos)) {
                case math_operator::MO_PLUS: {
                    nPos += 1;
                    const std::int64_t nRight = do_mult(nPos, nX);
                    nLeft = sum(nLeft, nRight);
                    break;
                }
                case math_operator::MO_MINUS: {
                    nPos += 1;
                    const std::int64_t nRight = do_mult(nPos, nX);
                    nLeft = rest(nLeft, nRight);
                    break;
                }
                default:
                    return nLeft;
            }
        } while (true);
    }

    std::int64_t math_parser::do_mult(
        std::size_t &nPos,
        const expvar &nX
    ) {
        depth_guard oDepthGuard(this);

        auto nLeft = do_pow(nPos, nX);

        do {
            switch (get_operator(nPos)) {
                case math_operator::MO_MULT: {
                    nPos += 1;
                    const std::int64_t nRight = do_pow(nPos, nX);
                    nLeft = mult(nLeft, nRight);
                    break;
                }
                case math_operator::MO_DIV: {
                    nPos += 1;
                    const std::int64_t nRight = do_pow(nPos, nX);
                    nLeft = div(nLeft, nRight);
                    break;
                }
                case math_operator::MO_MOD: {
                    nPos += 1;
                    const std::int64_t nRight = do_pow(nPos, nX);
                    nLeft = mod(nLeft, nRight);
                    break;
                }
                default:
                    return nLeft;
            }
        } while (true);
    }

    std::int64_t math_parser::do_pow(
        std::size_t &nPos,
        const expvar &nX
    ) {
        depth_guard oDepthGuard(this);

        auto nLeft = do_expr(nPos, nX);

        do {
            switch (get_operator(nPos)) {
                case math_operator::MO_POW: {
                    nPos += 1;
                    const std::int64_t nRight = do_expr(nPos, nX);
                    nLeft = pow(nLeft, nRight);
                    break;
                }
                default:
                    return nLeft;
            }
        } while (true);
    }

    std::int64_t math_parser::do_expr(
        std::size_t &nPos,
        const expvar &nX
    ) {
        bool bSign = false;

        switch (get_operator(nPos)) {
            case math_operator::MO_PLUS: nPos += 1;
                break;
            case math_operator::MO_MINUS: nPos += 1;
                bSign = true;
                break;
            default: break;
        }

        switch (get_operator(nPos)) {
            case math_operator::MO_OPEN_PARENTHESIS: {
                nPos += 1;
                const auto v = do_parentheses(nPos, nX);
                return bSign ? -v : v;
            }
            case math_operator::MO_FACTORIAL: {
                nPos += 1;
                const auto v = do_toplevel(nPos, nX);
                const auto f = factorial(v);
                return bSign ? -f : f;
            }
            case math_operator::MO_SIGN: {
                nPos += 1;
                const auto v = do_toplevel(nPos, nX);
                const auto s = v > 0 ? 1 : v < 0 ? -1 : 0;
                return bSign ? -s : s;
            }
            case math_operator::MO_ABS: {
                nPos += 1;
                const auto v = do_toplevel(nPos, nX);
                const auto a = v < 0 ? -v : v;
                return bSign ? -a : a;
            }
            case math_operator::MO_SUM: {
                nPos += 1;
                const auto v = do_list_sum(nPos, nX);
                return bSign ? -v : v;
            }
            case math_operator::MO_PRODUCT: {
                nPos += 1;
                const auto v = do_list_mult(nPos, nX);
                return bSign ? -v : v;
            }
            case math_operator::MO_NONE:
                break;
            default:
                throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        }

        if (nPos >= m_vTokens.size())
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);

        std::int64_t nValue = 0;

        if (nX.get_value(m_vTokens[nPos], nValue)) {
            nPos += 1;
            return bSign ? -nValue : nValue;
        }

        if (!is_number(m_vTokens[nPos]))
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_NOT_AN_INTEGER);

        nValue = std::stoll(m_vTokens[nPos]);
        nPos += 1;

        return bSign ? -nValue : nValue;
    }

    std::int64_t math_parser::do_parentheses(
        std::size_t &nPos,
        const expvar &nX
    ) {
        depth_guard oDepthGuard(this);

        const auto nContent = do_toplevel(nPos, nX);

        if (nPos >= m_vTokens.size() || m_vTokens[nPos] != ")"sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);

        nPos += 1;
        return nContent;
    }

    std::int64_t math_parser::do_list_sum(
        std::size_t &nPos,
        const expvar &nX
    ) {
        depth_guard oDepthGuard(this);

        if (nPos >= m_vTokens.size() || m_vTokens[nPos] != "("sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        const std::string_view sVar(m_vTokens[nPos]);
        if (!is_var(sVar))
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_INVALID_VARIABLE_NAME);

        expvar oVar = nX;
        nPos += 1;

        if (nPos >= m_vTokens.size() || m_vTokens[nPos] != ","sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        const auto nStart = do_toplevel(nPos, nX);

        if (m_vTokens[nPos] != ","sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        const auto nStep = do_toplevel(nPos, nX);

        if (m_vTokens[nPos] != ","sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        const auto nEnd = do_toplevel(nPos, nX);

        if (m_vTokens[nPos] != ","sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        if (nStep == 0 ||
            (nEnd >= nStart && nStep < 0) ||
            (nEnd <= nStart && nStep > 0)) {
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_SEQ_ITERATION_LOGIC);
        }

        std::int64_t nSum = 0;
        std::size_t nPosCopy = nPos;

        if (nStep > 0) {
            for (std::int64_t i = nStart; i <= nEnd; i = sum(i, nStep)) {
                oVar[sVar] = i;
                nPosCopy = nPos;
                nSum = sum(nSum, do_toplevel(nPosCopy, oVar));
            }
        } else {
            for (std::int64_t i = nStart; i >= nEnd; i = sum(i, nStep)) {
                oVar[sVar] = i;
                nPosCopy = nPos;
                nSum = sum(nSum, do_toplevel(nPosCopy, oVar));
            }
        }

        nPos = nPosCopy;

        if (nPos >= m_vTokens.size() || m_vTokens[nPos] != ")"sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);

        nPos += 1;
        return nSum;
    }

    std::int64_t math_parser::do_list_mult(
        std::size_t &nPos,
        const expvar &nX
    ) {
        depth_guard oDepthGuard(this);

        if (nPos >= m_vTokens.size() || m_vTokens[nPos] != "("sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        const std::string_view sVar(m_vTokens[nPos]);
        if (!is_var(sVar))
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_INVALID_VARIABLE_NAME);

        expvar oVar = nX;
        nPos += 1;

        if (m_vTokens[nPos] != ","sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        const auto nStart = do_toplevel(nPos, nX);

        if (m_vTokens[nPos] != ","sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        const auto nStep = do_toplevel(nPos, nX);

        if (m_vTokens[nPos] != ","sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        const auto nEnd = do_toplevel(nPos, nX);

        if (m_vTokens[nPos] != ","sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);
        nPos += 1;

        if (nStep == 0 ||
            (nEnd >= nStart && nStep < 0) ||
            (nEnd <= nStart && nStep > 0)) {
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_SEQ_ITERATION_LOGIC);
        }

        std::int64_t nProd = 1;
        std::size_t nPosCopy = nPos;

        if (nStep > 0) {
            for (std::int64_t i = nStart; i <= nEnd; i = sum(i, nStep)) {
                oVar[sVar] = i;
                nPosCopy = nPos;
                nProd = mult(nProd, do_toplevel(nPosCopy, oVar));
            }
        } else {
            for (std::int64_t i = nStart; i >= nEnd; i = sum(i, nStep)) {
                oVar[sVar] = i;
                nPosCopy = nPos;
                nProd = mult(nProd, do_toplevel(nPosCopy, oVar));
            }
        }

        nPos = nPosCopy;

        if (nPos >= m_vTokens.size() || m_vTokens[nPos] != ")"sv)
            throw math_error(shell_status::SHELL_CMD_ERROR_MATH_MALFORMED_EXPRESSION);

        nPos += 1;
        return nProd;
    }
}
