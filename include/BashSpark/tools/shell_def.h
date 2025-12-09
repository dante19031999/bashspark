/**
 * @file shell_def.h
 * @brief Defines and implements some hightly recurrent helper functions.
 *
 * Provides an interface to retrieve and set variables
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

#include <string>

namespace bs {
    /**
     * @brief Check whether a std::string represents a valid unsigned integer argument.
     *
     * A valid argument:
     * - is non-empty
     * - contains only digits ('0'–'9')
     * - contains at most 19 characters (limits of std::uint64_t)
     *
     * @param sArg String to validate.
     * @return true if valid; otherwise false.
     */
    constexpr bool is_arg(const std::string_view &sArg) {
        if (sArg.length() > 19)return false;
        for (const char cChar: sArg) {
            if (cChar < '0' || cChar > '9') return false;
        }
        return true;
    }

    /**
     * @brief Check whether a std::string represents a valid unsigned integer argument.
     *
     * A valid argument:
     * - is non-empty
     * - contains only digits ('0'–'9')
     * - contains at most 19 characters (limits of std::uint64_t)
     *
     * @param sArg String to validate.
     * @return true if valid; otherwise false.
     */
    inline bool is_arg(const std::string &sArg) {
        if (sArg.empty() || sArg.length() > 19)return false;
        for (const char cChar: sArg) {
            if (cChar < '0' || cChar > '9') return false;
        }
        return true;
    }

    /**
     * @brief Internal helper: check whether the given string_view is a valid variable name.
     *
     * A valid variable name:
     * - is non-empty
     * - starts with: underscore `_` or a letter `a–z`, `A–Z`
     * - subsequent characters may be:
     *     - underscore `_`
     *     - letters `a–z`, `A–Z`
     *     - digits `0–9`
     *
     * @param sVar String to validate.
     * @return true if the variable name is valid.
     */
    constexpr bool is_var(const std::string_view &sVar) {
        if (sVar.empty())return false;
        char cChar = sVar[0];
        if (
            cChar != '_'
            && (cChar < 'a' || cChar > 'z')
            && (cChar < 'A' || cChar > 'Z')
        ) { return false; }
        for (std::size_t i = 1; i < sVar.length(); ++i) {
            cChar = sVar[i];
            if (
                cChar != '_'
                && (cChar < 'a' || cChar > 'z')
                && (cChar < 'A' || cChar > 'Z')
                && (cChar < '0' || cChar > '9')
            ) { return false; }
        }
        return true;
    }

    /**
     * @brief Internal helper: check whether the given string_view is a valid variable name.
     *
     * A valid variable name:
     * - is non-empty
     * - starts with: underscore `_` or a letter `a–z`, `A–Z`
     * - subsequent characters may be:
     *     - underscore `_`
     *     - letters `a–z`, `A–Z`
     *     - digits `0–9`
     *
     * @param sVar String to validate.
     * @return true if the variable name is valid.
     */
    inline bool is_var(const std::string &sVar) {
        if (sVar.empty())return false;
        char cChar = sVar[0];
        if (
            cChar != '_'
            && (cChar < 'a' || cChar > 'z')
            && (cChar < 'A' || cChar > 'Z')
        ) { return false; }
        for (std::size_t i = 1; i < sVar.length(); ++i) {
            cChar = sVar[i];
            if (
                cChar != '_'
                && (cChar < 'a' || cChar > 'z')
                && (cChar < 'A' || cChar > 'Z')
                && (cChar < '0' || cChar > '9')
            ) { return false; }
        }
        return true;
    }

    /**
     * @brief Checks if the provided string is a valid representation of a number.
     *
     * This function verifies whether the input string can be interpreted as
     * a numerical value. It checks that the string is not empty, does not exceed
     * a length of 18 digits (limited by std::int64_t range), and contains only digits.
     * It also allows for an optional leading '+' or '-' sign.
     *
     * @param sArg A string view representing the input string to be checked.
     * @return True if the string is a valid number representation, false otherwise.
     */
    constexpr bool is_number(const std::string_view &sArg) {
        if (sArg.empty())return false;
        std::size_t i = 0;
        if (sArg[0] == '+' || sArg[0] == '-') {
            if (sArg.length() > 19)return false;
            ++i;
        } else {
            if (sArg.length() > 18)return false;
        }
        while (i < sArg.length()) {
            if (sArg[i] < '0' || sArg[i] > '9') return false;
            ++i;
        }
        return true;
    }
}
