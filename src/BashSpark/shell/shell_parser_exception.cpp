/**
 * @file shell_parser_exception.cpp
 * @brief Implements class shell_exception.
 *
 * This file contains the definition of the `shell_parser_exception` class,
 * which represents shell parsing errors.
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

#include "BashSpark/shell/shell_parser_exception.h"

namespace bs {
    namespace {
        /**
         * @brief Returns a string describing the error based on the shell status.
         *
         * @param nStatus The status code representing the shell execution state.
         * @return A string message corresponding to the shell status.
         */
        std::string errorMessage(const shell_status nStatus) {
            switch (nStatus) {
                case shell_status::SHELL_SUCCESS:
                    return "Success";
                case shell_status::SHELL_ERROR:
                    return "Generic error";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR:
                    return "Syntax error in command";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SIMPLE_QUOTES:
                    return "Unclosed simple quotes";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_DOUBLE_QUOTES:
                    return "Unclosed double quotes";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_BACK_QUOTES:
                    return "Unclosed back quotes";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_PARENTHESES:
                    return "Unclosed parentheses";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_BRACKETS:
                    return "Unclosed brackets";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SQR_BRACKETS:
                    return "Unclosed square brackets";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_SUBCOMMAND:
                    return "Unclosed subcommand";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNCLOSED_VARIABLE:
                    return "Unclosed variable";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_VARIABLE_NAME:
                    return "Invalid variable name";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_TOKEN:
                    return "Unexpected token";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNEXPECTED_EOF:
                    return "Unexpected end of file";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_ARG_OUT_OF_RANGE:
                    return "Argument out of range";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_EMPTY_BLOCK:
                    return "Empty block";
                case shell_status::SHELL_ERROR_BAD_ENCODING:
                    return "Bad encoding";
                case shell_status::SHELL_ERROR_COMMAND_NOT_FOUND:
                    return "Command not found";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_IF:
                    return "Syntax error: 'if' keyword is not finished.";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_THEN:
                    return "Syntax error: 'then' keyword is missing.";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_LOOP:
                    return "Syntax error: 'loop' keyword is not finished.";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_FOR:
                    return "Syntax error: 'for' keyword is not finished.";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_IN:
                    return "Syntax error: 'in' keyword is missing.";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_WHILE:
                    return "Syntax error: 'while' keyword is not finished.";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_UNFINISHED_KEYWORD_UNTIL:
                    return "Syntax error: 'until' keyword is not finished.";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_MISSING_KEYWORD_DO:
                    return "Syntax error: 'do' keyword is missing.";
                case shell_status::SHELL_ERROR_MAX_DEPTH_REACHED:
                    return "Maximum command nesting depth reached";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_FUNCTION_NAME:
                    return "Invalid function name";
                case shell_status::SHELL_ERROR_SYNTAX_ERROR_INVALID_FUNCTION_BODY:
                    return "Invalid function body";
                default:
                    return "Unknown error";
            }
        }

        /**
         * @brief Gets the line of text from a command string at a specific position.
         *
         * @param sCommand The command string to search within.
         * @param nPos The position index to extract the line from.
         * @return The line containing the given position or an empty string if out of bounds.
         */
        std::string getLineFromPosition(const std::string &sCommand, const std::size_t nPos) {
            if (nPos >= sCommand.size()) {
                return ""; // Return an empty string if position is out of bounds
            }

            // Start position for the line
            std::size_t start = sCommand.rfind('\n', nPos); // Find the previous newline character
            if (start == std::string::npos) {
                start = 0; // No newline found, start from the beginning
            } else {
                start++; // Move past the newline character
            }

            // End position for the line
            std::size_t end = sCommand.find('\n', nPos); // Find the next newline character
            if (end == std::string::npos) {
                end = sCommand.size(); // No newline found, go to end of string
            }

            // Return the substring that represents the line
            return sCommand.substr(start, end - start);
        }

        /**
         * @brief Gets the code point (character index) at a specific byte position in a UTF-8 encoded string.
         *
         * @param sCommand The command string to analyze.
         * @param nPos The byte index in the string.
         * @return The character index (code point) corresponding to the specified byte position.
         * @warning Returns nPos if any error occurred
         */
        std::size_t getCodePointAtByte(const std::string &sCommand, const size_t nPos) {
            if (nPos >= sCommand.size()) return nPos;

            std::size_t nByteIndex = 0;
            std::size_t nCharIndex = 0;

            while (nByteIndex < sCommand.size()) {
                // Get char
                const auto cFirstByte = static_cast<unsigned char>(sCommand[nByteIndex]);
                std::size_t nBytesProcessed = 0;

                // Determine the number of bytes in the UTF-8 character
                if ((cFirstByte & 0x80) == 0) {
                    // 1-byte character
                    nBytesProcessed = 1;
                } else if ((cFirstByte & 0xE0) == 0xC0) {
                    // 2-byte character
                    nBytesProcessed = 2;
                } else if ((cFirstByte & 0xF0) == 0xE0) {
                    // 3-byte character
                    nBytesProcessed = 3;
                } else if ((cFirstByte & 0xF8) == 0xF0) {
                    // 4-byte character
                    nBytesProcessed = 4;
                } else {
                    return nPos;
                }

                // Check if the current character starts at the specified byte index
                if (nPos >= nByteIndex && nPos < nByteIndex + nBytesProcessed) {
                    return nCharIndex;
                }

                // Move to the next character
                nCharIndex++;
                nByteIndex += nBytesProcessed;
            }

            throw std::out_of_range("Byte index is out of range");
        }

        /**
         * @brief Constructs a detailed error message based on the shell status, command, and position.
         *
         * @param nStatus The status code representing the shell execution state.
         * @param sCommand The command string associated with the error.
         * @param nPos The byte position where the error occurred.
         * @return A formatted error message string.
         */
        std::string makeMessage(
            const shell_status nStatus,
            const std::string &sCommand,
            const std::size_t nPos
        ) {
            const std::string sLine = getLineFromPosition(sCommand, nPos);
            const auto nCodePoint = getCodePointAtByte(sLine, nPos);
            const auto nAbsoluteCodePoint = getCodePointAtByte(sCommand, nPos);
            return errorMessage(nStatus)
                   + "\n" + sLine
                   + "\n" + std::string(nCodePoint, ' ') + "^~~~"
                   + "\nCode point: " + std::to_string(nAbsoluteCodePoint)
                   + "\nByte: " + std::to_string(nPos) + "\n";
        }
    }

    shell_parser_exception::shell_parser_exception(
        const shell_status nStatus,
        std::string sCommand,
        const std::size_t nPos
    )
        : std::runtime_error(makeMessage(
              nStatus, sCommand, nPos
          )),
          m_nStatus(nStatus),
          m_sCommand(std::move(sCommand)),
          m_nPos(nPos) {
    }
}
