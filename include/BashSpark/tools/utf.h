/**
 * @file utf.h
 * @brief Provides simple tools for utf string manipulation.
 *
 * @author Dante Doménech Martínez
 * @date 21/11/25
 *
 * @copyright MIT License
 *
 * This file is part of BashSpark.
 * Copyright (c) 2025 Dante Doménech Martínez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <iomanip>
#include <regex>
#include <cstdint>
#include <iostream>

#include "BashSpark/tools/fakestream.h"

namespace bs {
    /**
     * @brief Combines high and low UTF-16 surrogates into a UTF-32 code point.
     *
     * @param high The high surrogate (in the range 0xD800 to 0xDBFF).
     * @param low The low surrogate (in the range 0xDC00 to 0xDFFF).
     * @return The corresponding UTF-32 code point.
     */
    constexpr char32_t combine_surrogates(const char16_t high, const char16_t low) {
        return ((static_cast<char32_t>(high) - 0xD800) << 10
                | static_cast<char32_t>(low) - 0xDC00) + 0x10000;
    }

    /**
     * @brief Converts a UTF-32 character to a hexadecimal string representation.
     *
     * @param cChar The UTF-32 character to convert.
     * @param nLength The number of hex digits to include (1 for byte, 2 for UTF-16, 4 for UTF-32).
     * @return A string representing the character in hexadecimal format.
     */
    inline std::string to_hex_string(const char32_t cChar, const std::size_t nLength) {
        std::ostringstream oStream;
        if (nLength == 1) oStream.write("\\x", 2);
        if (nLength == 2) oStream.write("\\u", 2);
        if (nLength == 4) oStream.write("\\U", 2);
        oStream << std::hex << std::setfill('0')
                << std::setw(static_cast<std::ostream::int_type>(nLength))
                << static_cast<std::uint32_t>(cChar);
        return oStream.str();
    }

    /**
     * @brief Writes a UTF-32 character to an output stream in UTF-8 encoding.
     *
     * @param oStream The output stream to write to.
     * @param cChar The UTF-32 character to encode and write.
     *
     * This function encodes the character based on its value, handling
     * the appropriate byte sequence for UTF-8.
     */
    inline void write_char32_t(ofakestream &oStream, const char32_t cChar) {
        if (cChar <= 0x7F) {
            // 1 byte (ASCII range)
            oStream.put(static_cast<char>(cChar));
        } else if (cChar <= 0x7FF) {
            // 2 bytes
            oStream.put(static_cast<char>(cChar >> 6 | 0xC0));
            oStream.put(static_cast<char>(cChar & 0x3F | 0x80));
        } else if (cChar <= 0xFFFF) {
            // 3 bytes
            oStream.put(static_cast<char>(cChar >> 12 | 0xE0));
            oStream.put(static_cast<char>(cChar >> 6 & 0x3F | 0x80));
            oStream.put(static_cast<char>(cChar & 0x3F | 0x80));
        } else if (cChar <= 0x10FFFF) {
            // 4 bytes
            oStream.put(static_cast<char>(cChar >> 18 | 0xF0));
            oStream.put(static_cast<char>(cChar >> 12 & 0x3F | 0x80));
            oStream.put(static_cast<char>(cChar >> 6 & 0x3F | 0x80));
            oStream.put(static_cast<char>(cChar & 0x3F | 0x80));
        }
    }

    /**
     * @brief Converts a UTF-32 character to a UTF-8 encoded string.
     *
     * @param cChar The UTF-32 character to convert.
     * @return A UTF-8 encoded string representation of the character.
     */
    inline std::string write_char32_t(const char32_t cChar) {
        ofakestream oStream;
        write_char32_t(oStream, cChar);
        return oStream.str();
    }

    /**
     * @brief Parses a UTF-n encoded character from an input stream.
     *
     * @param oIstream The input stream to read from.
     * @param nCount The number of hex digits to read (1 for UTF-8, 2 for UTF-16, 4 for UTF-32).
     * @param cResult Parse UTF-32 character.
     * @return Whether a UTF-32 character could be successfully parsed.
     *
     * This function reads the specified number of characters, checks for valid hex format,
     * and handles surrogate pairs as needed.
     *
     * @throw bash_utf_error If the input is invalid or not properly encoded.
     */
    inline bool parse_utf(ifakestream &oIstream, const std::size_t nCount, char32_t &cResult) {
        // Read
        std::string sValue(nCount * 2, '\0');
        oIstream.read(sValue.data(), nCount * 2);

        // Check hex
        static std::regex oRegex("^[0-9A-Fa-f]+$");
        if (!std::regex_match(sValue, oRegex))
            return false;
        auto cChar = static_cast<char32_t>(std::stoull(sValue, nullptr, 16));

        switch (nCount) {
            case 1: {
                if (cChar > 0x7F)
                    return false;
                break;
            }
            case 2: {
                // Deal with surrogates
                if (cChar >= 0xD800 && cChar <= 0xDBFF) {
                    if (oIstream.get() != '\\') return false;
                    if (oIstream.get() != 'u') return false;

                    // Load other surrogate and check regex
                    oIstream.read(sValue.data(), nCount * 2);
                    if (!std::regex_match(sValue, oRegex))
                        return false;

                    const auto cLow = static_cast<char32_t>(std::stoull(sValue, nullptr, 16));
                    if (cLow < 0xDC00 || cLow > 0xDFFF)
                        return false;

                    cChar = combine_surrogates(
                        static_cast<char16_t>(cChar),
                        static_cast<char16_t>(cLow)
                    );
                } else if (cChar >= 0xDC00) {
                    // High surrogates and above
                    return false;
                }
                break;
            }

            case 4: {
                if ((cChar >= 0xD800 && cChar <= 0xDFFF) || cChar > 0x10FFFF)
                    return false;
                break;
            }

            default: break;
        }

        cResult = cChar;
        return true;
    }
}
