/**
 * @file shell_keyword.h
 * @brief Defines enum bs::shell_keyword.
 *
 * This file contains the definition of the `bs::shell_keyword` enum,
 * which provides helper methods for class `shell`.
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

#include <type_traits>

namespace bs {
    /**
     * @enum shell_keyword
     * @brief Enumeration of shell keywords for scripting.
     */
    enum class shell_keyword {
        SK_NONE = 0x0, ///< No keyword (default value).
        SK_FUNCTION = 1 << 0, ///< Start of a function definition.
        SK_IF = 1 << 1, ///< Beginning of an if conditional statement.
        SK_THEN = 1 << 2, ///< Block that executes if the 'if' condition is true.
        SK_ELSE = 1 << 3, ///< Alternative block for when the 'if' condition is false.
        SK_ELIF = 1 << 4, ///< Additional condition in an if-else chain.
        SK_FI = 1 << 5, ///< End of an if-else construct.
        SK_FOR = 1 << 6, ///< Start of a for loop.
        SK_IN = 1 << 7, ///< Collection specified in a for loop.
        SK_WHILE = 1 << 8, ///< Start of a while loop.
        SK_UNTIL = 1 << 9, ///< Loop continues until a condition is met.
        SK_DO = 1 << 10, ///< Beginning of a block for loops and conditionals.
        SK_DONE = 1 << 11, ///< End of a do block or loop.
        SK_CONTINUE = 1 << 12, ///< Continue statement.
        SK_BREAK = 1 << 13, ///< Break staement.
        SK_IF_DELIMITER = SK_ELSE | SK_ELIF | SK_FI ///< End of a if block.
    };

    /**
     * @enum parse_mode
     * @brief Enumeration for different parsing modes.
     *
     * This enum defines various modes for the parsing process, allowing for
     * specific behaviors depending on the mode set.
     */
    enum class parse_mode {
        PM_NORMAL = 0x0, ///< Normal parsing mode.
        PM_BACKQUOTE = 1 << 0, ///< Backquote parsing mode.
        PM_LOOP = 1 << 1, ///< Loop parsing mode.
        PM_FUNCTION_NAME = 1 << 2, // Function name parsing mode
        PM_BACKQUOTE_LOOP = PM_BACKQUOTE & PM_LOOP ///< Combined mode for handling both backquote and looping.
    };

    /**
     * @brief Checks if the bitwise and of two `parse_mode` is 0.
     * @param nItem Left parse mode
     * @param nSet Right parse mode
     * @return Whether the intersection is null
     */
    constexpr bool has(const parse_mode nItem, const parse_mode nSet) {
        return (
                   static_cast<std::underlying_type_t<parse_mode>>(nItem) &
                   static_cast<std::underlying_type_t<parse_mode>>(nSet)
               ) != 0;
    }

    /**
     * @brief Checks if the bitwise and of two `shell_keyword` is 0.
     * @param nItem Left keyword
     * @param nSet Right keyword
     * @return Whether the intersection is null
     */
    constexpr bool has(const shell_keyword nItem, const shell_keyword nSet) {
        return (
                   static_cast<std::underlying_type_t<shell_keyword>>(nItem) &
                   static_cast<std::underlying_type_t<shell_keyword>>(nSet)
               ) != 0;
    }
}

/**
 * @brief Bitwise OR operator for shell_keyword.
 * @param nLeft The left operand.
 * @param nRight The right operand.
 * @return The result of the bitwise OR as a shell_keyword.
 */
constexpr bs::shell_keyword operator|(const bs::shell_keyword nLeft, const bs::shell_keyword nRight) {
    return static_cast<bs::shell_keyword>(
        static_cast<std::underlying_type_t<bs::shell_keyword>>(nLeft) |
        static_cast<std::underlying_type_t<bs::shell_keyword>>(nRight));
}

/**
 * @brief Bitwise AND operator for shell_keyword.
 * @param nLeft The left operand.
 * @param nRight The right operand.
 * @return The result of the bitwise AND as a shell_keyword.
 */
constexpr bs::shell_keyword operator&(const bs::shell_keyword nLeft, const bs::shell_keyword nRight) {
    return static_cast<bs::shell_keyword>(
        static_cast<std::underlying_type_t<bs::shell_keyword>>(nLeft) &
        static_cast<std::underlying_type_t<bs::shell_keyword>>(nRight));
}


/**
 * @brief Bitwise XOR operator for shell_keyword.
 * @param nLeft The left operand.
 * @param nRight The right operand.
 * @return The result of the bitwise XOR as a shell_keyword.
 */
constexpr bs::shell_keyword operator^(const bs::shell_keyword nLeft, const bs::shell_keyword nRight) {
    return static_cast<bs::shell_keyword>(
        static_cast<std::underlying_type_t<bs::shell_keyword>>(nLeft) ^
        static_cast<std::underlying_type_t<bs::shell_keyword>>(nRight));
}

/**
 * @brief Bitwise OR assignment operator for shell_keyword.
 * @param nLeft The left operand, which will be modified.
 * @param nRight The right operand.
 * @return A reference to the modified left operand.
 */
inline bs::shell_keyword &operator|=(bs::shell_keyword &nLeft, const bs::shell_keyword nRight) {
    return nLeft = nLeft | nRight;
}

/**
 * @brief Bitwise AND assignment operator for shell_keyword.
 * @param nLeft The left operand, which will be modified.
 * @param nRight The right operand.
 * @return A reference to the modified left operand.
 */
inline bs::shell_keyword &operator&=(bs::shell_keyword &nLeft, const bs::shell_keyword nRight) {
    return nLeft = nLeft & nRight;
}

/**
 * @brief Bitwise XOR assignment operator for shell_keyword.
 * @param nLeft The left operand, which will be modified.
 * @param nRight The right operand.
 * @return A reference to the modified left operand.
 */
inline bs::shell_keyword &operator^=(bs::shell_keyword &nLeft, const bs::shell_keyword nRight) {
    return nLeft = nLeft ^ nRight;
}

/**
 * @brief Bitwise OR operator for parse_mode.
 * @param nLeft The left operand.
 * @param nRight The right operand.
 * @return The result of the bitwise OR as a parse_mode.
 */
constexpr bs::parse_mode operator|(const bs::parse_mode nLeft, const bs::parse_mode nRight) {
    return static_cast<bs::parse_mode>(
        static_cast<std::underlying_type_t<bs::parse_mode>>(nLeft) |
        static_cast<std::underlying_type_t<bs::parse_mode>>(nRight));
}

/**
 * @brief Bitwise AND operator for parse_mode.
 * @param nLeft The left operand.
 * @param nRight The right operand.
 * @return The result of the bitwise AND as a parse_mode.
 */
constexpr bs::parse_mode operator&(const bs::parse_mode nLeft, const bs::parse_mode nRight) {
    return static_cast<bs::parse_mode>(
        static_cast<std::underlying_type_t<bs::parse_mode>>(nLeft) &
        static_cast<std::underlying_type_t<bs::parse_mode>>(nRight));
}


/**
 * @brief Bitwise XOR operator for parse_mode.
 * @param nLeft The left operand.
 * @param nRight The right operand.
 * @return The result of the bitwise XOR as a parse_mode.
 */
constexpr bs::parse_mode operator^(const bs::parse_mode nLeft, const bs::parse_mode nRight) {
    return static_cast<bs::parse_mode>(
        static_cast<std::underlying_type_t<bs::parse_mode>>(nLeft) ^
        static_cast<std::underlying_type_t<bs::parse_mode>>(nRight));
}

/**
 * @brief Bitwise OR assignment operator for parse_mode.
 * @param nLeft The left operand, which will be modified.
 * @param nRight The right operand.
 * @return A reference to the modified left operand.
 */
inline bs::parse_mode &operator|=(bs::parse_mode &nLeft, const bs::parse_mode nRight) {
    return nLeft = nLeft | nRight;
}

/**
 * @brief Bitwise AND assignment operator for parse_mode.
 * @param nLeft The left operand, which will be modified.
 * @param nRight The right operand.
 * @return A reference to the modified left operand.
 */
inline bs::parse_mode &operator&=(bs::parse_mode &nLeft, const bs::parse_mode nRight) {
    return nLeft = nLeft & nRight;
}

/**
 * @brief Bitwise XOR assignment operator for parse_mode.
 * @param nLeft The left operand, which will be modified.
 * @param nRight The right operand.
 * @return A reference to the modified left operand.
 */
inline bs::parse_mode &operator^=(bs::parse_mode &nLeft, const bs::parse_mode nRight) {
    return nLeft = nLeft ^ nRight;
}
