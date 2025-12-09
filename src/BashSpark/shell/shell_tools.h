/**
 * @file shell_tools.h
 * @brief Provides some simple tools used on command expansion.
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
#include <vector>

#include "BashSpark/tools/utf.h"
#include "BashSpark/shell/shell_parser.h"

#ifdef __WIN64
#include <windows.h>
#elif defined(__posix__) || defined(__APPLE__)
#include <unistd.h>
#endif

namespace bs {
    /**
     * @brief Splits a string into a vector
     * @param oString String to split
     * @return Vector resulting of string split
     */
    inline std::vector<std::string> split_string(const std::string_view &oString) {
        std::vector<std::string> vStrings;
        std::size_t nStart = 0;
        std::size_t nLength = 0;
        for (std::size_t i = 0; i < oString.size(); ++i) {
            if (
                const char cChar = oString[i];
                cChar == ' ' || cChar == '\n' || cChar == '\t'
            ) {
                // Append pending
                if (nLength != 0)
                    vStrings.emplace_back(oString.substr(nStart, nLength));
                // Reset buffer
                nStart = i + 1;
                nLength = 0;
            } else {
                // Continue searching new end
                ++nLength;
            }
        }
        // Append remainder
        if (nLength != 0)
            vStrings.emplace_back(oString.substr(nStart, nLength));
        return vStrings;
    }

    /**
     * @brief Splits a string into a vector
     * @param vStrings Vector of strings where to split
     * @param oString String to split
     */
    inline void split_string(
        std::vector<std::string> &vStrings,
        const std::string_view &oString
    ) {
        std::size_t nStart = 0;
        std::size_t nLength = 0;
        for (std::size_t i = 0; i < oString.size(); ++i) {
            if (
                const char cChar = oString[i];
                cChar == ' ' || cChar == '\n' || cChar == '\t'
            ) {
                // Append pending
                if (nLength != 0)
                    vStrings.emplace_back(oString.substr(nStart, nLength));
                // Reset buffer
                nStart = i + 1;
                nLength = 0;
            } else {
                // Continue searching new end
                ++nLength;
            }
        }
        // Append remainder
        if (nLength != 0)
            vStrings.emplace_back(oString.substr(nStart, nLength));
    }

    /**
     * @brief Moves appends the contents of the origin vector into the destination vector
     * @param vDestiny Destination vector
     * @param vOrigin Origin vector
     */
    inline void insert_vector(std::vector<std::string> &vDestiny, std::vector<std::string> vOrigin) {
        vDestiny.insert(
            vDestiny.end(),
            std::make_move_iterator(vOrigin.begin()),
            std::make_move_iterator(vOrigin.end())
        );
    }

    /**
     * @brief Writes the contents of a vector<string> on an output stream
     * @param oOstream Output stream to write
     * @param vOrigin Vector to write
     */
    inline void concat_vector(ofakestream &oOstream, const std::vector<std::string> &vOrigin) {
        if (!vOrigin.empty()) {
            oOstream << vOrigin[0];
            for (std::size_t i = 1; i < vOrigin.size(); ++i) {
                oOstream.put(' ');
                oOstream << vOrigin[i];
            }
        }
    }

    /**
     * @brief Gets the PID process of the shell
     * On non recognized platforms returns 0.
     * @return PID process of the shell
     */
    inline std::int64_t get_pid() {
#ifdef __WIN64
        return GetCurrentProcessId();
#elif defined(__posix__) || defined(__APPLE__)
        return getpid();
#else
        return 0;
#endif
    }

    /**
     * @brief Identifies shell special keywords
     * @param oString Keyword to identify
     * @return Keyword code
     */
    inline shell_keyword get_keyword_id(const std::string_view &oString) {
        const static std::unordered_map<std::string_view, shell_keyword> s_mKeywords = {
            {"function", shell_keyword::SK_FUNCTION},
            {"if", shell_keyword::SK_IF},
            {"then", shell_keyword::SK_THEN},
            {"else", shell_keyword::SK_ELSE},
            {"elif", shell_keyword::SK_ELIF},
            {"fi", shell_keyword::SK_FI},
            {"for", shell_keyword::SK_FOR},
            {"in", shell_keyword::SK_IN},
            {"while", shell_keyword::SK_WHILE},
            {"until", shell_keyword::SK_UNTIL},
            {"do", shell_keyword::SK_DO},
            {"done", shell_keyword::SK_DONE},
            {"continue", shell_keyword::SK_CONTINUE},
            {"break", shell_keyword::SK_BREAK},
        };
        const auto pIter = s_mKeywords.find(oString);
        if (pIter == s_mKeywords.end()) return shell_keyword::SK_NONE;
        return pIter->second;
    }

    /**
     * @brief Identifies shell special keywords
     * @param oString Keyword to identify
     * @return Keyword code
     */
    inline shell_keyword get_keyword_id(const std::string &oString) {
        const static std::map<std::string, shell_keyword> s_mKeywords = {
            {"function", shell_keyword::SK_FUNCTION},
            {"if", shell_keyword::SK_IF},
            {"then", shell_keyword::SK_THEN},
            {"else", shell_keyword::SK_ELSE},
            {"elif", shell_keyword::SK_ELIF},
            {"fi", shell_keyword::SK_FI},
            {"for", shell_keyword::SK_FOR},
            {"in", shell_keyword::SK_IN},
            {"while", shell_keyword::SK_WHILE},
            {"until", shell_keyword::SK_UNTIL},
            {"do", shell_keyword::SK_DO},
            {"done", shell_keyword::SK_DONE}
        };
        const auto pIter = s_mKeywords.find(oString);
        if (pIter == s_mKeywords.end()) return shell_keyword::SK_NONE;
        return pIter->second;
    }
}
