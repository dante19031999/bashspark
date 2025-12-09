/**
 * @file fakestream.h
 * @brief Generic input and output stream classes with character types.
 *
 * This header file defines the implementation of the `basic_ifakestream` and
 * `basic_ofakestream` classes, which serve as generic input and output
 * streams for various character types (e.g., char, wchar_t, char8_t,
 * char16_t, char32_t). It provides functionalities similar to standard
 * C++ streams but operates directly on raw character data.
 *
 * @note The classes include memory management for dynamically allocated
 * buffers and provide mechanisms for reading from and writing to these
 * buffers efficiently. The implementation utilizes inline functions for
 * performance optimizations.
 *
 * @date Created on 21/11/25
 * @author Dante Doménech Martínez
 *
 * @warning Ensure that the appropriate character type is used when
 * instantiating the stream classes. This file requires C++11 or later.
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <string_view>
#include <new>

#ifndef ALWAYS_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define ALWAYS_INLINE
#endif
#endif

namespace bs {
    /**
     * @brief A class for input stream behavior with a character type.
     * @tparam char_t The character type (e.g., char, wchar_t).
     */
    template<typename char_t>
    class basic_ifakestream {
    public:
        /// Type traits for the character type.
        using traits_type = std::char_traits<char_t>;
        /// Fundamental character type.
        using char_type = traits_type::char_type;
        /// Type for representing character values.
        using int_type = traits_type::int_type;

        /// End-of-file marker.
        static constexpr auto EOF_VALUE = traits_type::eof();

        /**
         * @brief Constructs an empty input stream.
         */
        ALWAYS_INLINE basic_ifakestream()
            : m_nPos(0), m_pData(nullptr), m_nSize(0) {
        }

        /**
         * @brief Constructs an input stream with data and size.
         * @param pData Pointer to the data.
         * @param nSize Size of the data.
         */
        ALWAYS_INLINE basic_ifakestream(const char *const pData, const std::size_t nSize)
            : m_nPos(0), m_pData(pData), m_nSize(nSize) {
        }

        /**
         * @brief Constructs an input stream with string view.
         * @param sText String view.
         */
        ALWAYS_INLINE basic_ifakestream(const std::string_view &sText)
            : m_nPos(0), m_pData(sText.data()), m_nSize(sText.length()) {
        }

        // Deleted copy and move constructors and assignment operators.
        basic_ifakestream(const basic_ifakestream &) = delete;

        basic_ifakestream(basic_ifakestream &&) = delete;

        basic_ifakestream &operator=(const basic_ifakestream &) = delete;

        basic_ifakestream &operator=(basic_ifakestream &&) = delete;

        /**
         * @brief Retrieves the next character character from the stream and moves to the next.
         * @return The next character or EOF_VALUE if at end of stream.
         */
        ALWAYS_INLINE int_type get() noexcept {
            auto nChar = m_nPos < m_nSize ? m_pData[m_nPos] : EOF_VALUE;
            this->m_nPos += 1;
            return nChar;
        }

        /**
         * @brief Retrieves the next character from the stream. Does not increment stream position.
         * @return The next character or EOF_VALUE if at end of stream.
         */
        ALWAYS_INLINE int_type peek() noexcept {
            return m_nPos < m_nSize ? m_pData[m_nPos] : EOF_VALUE;
        }

        /**
         * @brief Retrieves the previous character from the stream. Does not move stream position.
         * @return The next character or EOF_VALUE if at beggining of stream.
         */
        ALWAYS_INLINE int_type prev() noexcept {
            return m_nPos > 0 ? m_nPos <= m_nSize ? m_pData[m_nPos - 1] : EOF_VALUE : EOF_VALUE;
        }

        /**
        * @brief Returns the last read character to the stream.
        */
        ALWAYS_INLINE void put_back() noexcept {
            if (m_nPos > 0)--m_nPos;
        }

        /**
         * @brief Reads a specified number of characters into a buffer.
         * @param pBuffer Pointer to the buffer for storing data.
         * @param nCount Number of characters to read.
         * @return Number of characters actually read.
         */
        ALWAYS_INLINE std::size_t read(char_type *const pBuffer, const std::size_t nCount) noexcept {
            const std::size_t nRead = m_nPos + nCount > m_nSize
                                          ? nCount + m_nPos - m_nSize
                                          : nCount;
            std::memcpy(pBuffer, this->m_pData + this->m_nPos, nRead * sizeof(char_type));
            this->m_nPos += nRead;
            return nRead;
        }

        /**
         * @brief Checks if the end of the stream has been reached.
         * @return True if at end of stream, otherwise false.
         */
        [[nodiscard]] ALWAYS_INLINE bool eof() const noexcept {
            return this->m_nPos >= m_nSize;
        }

        /**
         * @brief Gets the current position in the stream.
         * @return Current position.
         */
        [[nodiscard]] ALWAYS_INLINE std::size_t tell() const noexcept {
            return this->m_nPos;
        }

        /**
         * @brief Seeks to a specified position in the stream.
         * @param pos Position to seek to.
         */
        ALWAYS_INLINE void seek(const std::size_t pos) noexcept {
            this->m_nPos = pos;
        }

        /**
        * @brief Gets the stream data size.
        * @return The stream data size.
        */
        [[nodiscard]] ALWAYS_INLINE std::size_t size() const noexcept {
            return this->m_nSize;
        }

        /**
         * @brief Gets a read-only view of the stream data.
         * @return A string view of the data.
         */
        [[nodiscard]] ALWAYS_INLINE std::basic_string_view<char_type> view() const noexcept {
            return {this->m_pData, this->m_nSize};
        }

        /**
         * @brief Gets a read-only view of the stream data.
         * @return A string view of the data.
         */
        [[nodiscard]] ALWAYS_INLINE std::basic_string_view<char_type> sub_view(
            const std::size_t nBegin, std::size_t nLength) const noexcept {
            nLength = nBegin + nLength > m_nSize
                          ? nBegin + nLength - m_nSize
                          : nLength;
            return {this->m_pData + nBegin, nLength};
        }

        /**
         * @brief Converts the data to a string.
         * @return A string representation of the data.
         */
        [[nodiscard]] ALWAYS_INLINE std::basic_string<char_type> str() const {
            return {this->m_pData, this->m_nSize};
        }

        /**
         * @brief Gets a read-only view of the stream remaining data.
         * @return A string view of the data.
         */
        [[nodiscard]] ALWAYS_INLINE std::basic_string_view<char_type> remaining_view() const noexcept {
            if (this->m_nPos >= this->m_nSize)return "";
            return {this->m_pData + this->m_nPos, this->m_nSize - this->m_nPos};
        }

        /**
         * @brief Converts the remaining data to a string.
         * @return A string representation of the data.
         */
        [[nodiscard]] ALWAYS_INLINE std::basic_string<char_type> remaining_str() const {
            if (this->m_nPos >= this->m_nSize)return "";
            return {this->m_pData + this->m_nPos, this->m_nSize - this->m_nPos};
        }

    private:
        std::size_t m_nPos; /// Current position in the stream.
        const char_type *const m_pData; /// Pointer to the data.
        const std::size_t m_nSize; /// Size of the data.
    };

    /// Alias for basic_ifakestream with char.
    using ifakestream = basic_ifakestream<char>;
    /// Alias for basic_ifakestream with wchar_t.
    using wifakestream = basic_ifakestream<wchar_t>;
    /// Alias for basic_ifakestream with char8_t.
    using u8ifakestream = basic_ifakestream<char8_t>;
    /// Alias for basic_ifakestream with char16_t.
    using u16ifakestream = basic_ifakestream<char16_t>;
    /// Alias for basic_ifakestream with char32_t.
    using u32ifakestream = basic_ifakestream<char32_t>;

    /**
     * @brief A class for output stream behavior with a character type.
     * @tparam char_t The character type (e.g., char, wchar_t).
     */
    template<typename char_t>
    class basic_ofakestream {
    public:
        /// Type traits for the character type.
        using traits_type = std::char_traits<char_t>;
        /// Fundamental character type.
        using char_type = traits_type::char_type;
        /// Type for representing character values.
        using int_type = traits_type::int_type;

        /// Default buffer size.
        static constexpr auto DEFAULT_BUFFER_SIZE = 64;
        /// End-of-file marker.
        static constexpr auto EOF_VALUE = traits_type::eof();

        /**
         * @brief Constructs an output stream with a default buffer.
         */
        ALWAYS_INLINE basic_ofakestream()
            : m_pData(static_cast<char_type *>(
                  std::malloc(DEFAULT_BUFFER_SIZE * sizeof(char_type))
              )),
              m_nSize(DEFAULT_BUFFER_SIZE),
              m_nPos(0) {
            if (this->m_pData == nullptr) throw std::bad_alloc();
        }

        // Deleted copy and move constructors and assignment operators.
        basic_ofakestream(const basic_ofakestream &) = delete;

        basic_ofakestream(basic_ofakestream &&) = delete;

        basic_ofakestream &operator=(const basic_ofakestream &) = delete;

        basic_ofakestream &operator=(basic_ofakestream &&) = delete;

        /**
         * @brief Destructor that frees allocated memory.
         */
        ALWAYS_INLINE ~basic_ofakestream() {
            std::free(this->m_pData);
        }

        /**
         * @brief Writes a single character to the stream.
         * @param cChar The character to write.
         */
        ALWAYS_INLINE void put(const char_type cChar) {
            if (m_nPos + 1 >= m_nSize) realloc(m_nPos + 1);
            m_pData[m_nPos++] = cChar;
        }

        /**
         * @brief Writes a block of data to the stream.
         * @param pData Pointer to the data to write.
         * @param nLength Length of the data to write.
         */
        ALWAYS_INLINE void write(const char_type *const pData, const std::size_t nLength) {
            if (m_nPos + nLength >= m_nSize) realloc(m_nPos + nLength);
            std::memcpy(this->m_pData + this->m_nPos, pData, nLength * sizeof(char_type));
            this->m_nPos += nLength;
        }

        /**
         * @brief Checks if the stream is empty.
         * @return True if the stream is empty, otherwise false.
         */
        [[nodiscard]] ALWAYS_INLINE bool empty() const noexcept {
            return this->m_nPos == 0;
        }

        /**
        * @brief Gets the stream size
        * @return The stream size
        */
        [[nodiscard]] ALWAYS_INLINE std::size_t size() const noexcept {
            return this->m_nPos;
        }

        /**
         * @brief Clears the stream by resetting the position.
         */
        ALWAYS_INLINE void clear() noexcept {
            this->m_nPos = 0;
        }

        /**
         * @brief Writes a string to the stream using the << operator.
         * @param sString The string to write.
         */
        ALWAYS_INLINE void operator<<(const std::basic_string<char_type> &sString) {
            const std::size_t nLength = sString.length();
            if (m_nPos + nLength >= m_nSize) realloc(m_nPos + nLength);
            std::memcpy(this->m_pData + this->m_nPos, sString.data(), nLength * sizeof(char_type));
            this->m_nPos += nLength;
        }

        /**
         * @brief Writes a string view to the stream using the << operator.
         * @param sString The string to write.
         */
        ALWAYS_INLINE void operator<<(const std::basic_string_view<char_type> &sString) {
            const std::size_t nLength = sString.length();
            if (m_nPos + nLength >= m_nSize) realloc(m_nPos + nLength);
            std::memcpy(this->m_pData + this->m_nPos, sString.data(), nLength * sizeof(char_type));
            this->m_nPos += nLength;
        }

        /**
         * @brief Gets a read-only view of the current state of the stream.
         * @return A string view of the data.
         */
        [[nodiscard]] ALWAYS_INLINE std::basic_string_view<char_type> view() const noexcept {
            return {this->m_pData, m_nPos};
        }

        /**
         * @brief Converts the current data to a string.
         * @return A string representation of the data.
         */
        [[nodiscard]] ALWAYS_INLINE std::basic_string<char_type> str() const {
            return {this->m_pData, m_nPos};
        }

        /**
         * @brief Resets the stream and returns the current data as a string.
         * @return A string representation of the data and resets the stream.
         */
        [[nodiscard]] ALWAYS_INLINE std::basic_string<char_type> str_reset() {
            std::basic_string<char_type> sResult{this->m_pData, m_nPos};
            this->m_nPos = 0;
            return sResult;
        }

        /**
        * @brief Converts a portion of the current data to a string.
        * @note Only the part of the range that falls within the data will be copied
        * @param nBegin Begin point of the data
        * @param nLength Length of the data
        * @return A substring of the current data
        */
        [[nodiscard]] ALWAYS_INLINE std::basic_string<char_type> sub_str(
            const std::size_t nBegin, std::size_t nLength) const noexcept {
            nLength = nBegin + nLength >= m_nSize
                          ? nBegin + nLength - m_nSize
                          : nLength;
            return {this->m_pData + nBegin, nLength};
        }

    private:
        /// Pointer to the allocated buffer.
        char_type *m_pData;
        /// Size of the allocated buffer.
        std::size_t m_nSize;
        /// Current position in the buffer.
        std::size_t m_nPos;

        /**
         * @brief Reallocates the buffer to a new size.
         * @param nRequestedSize The size requested for the new buffer.
         */
        void realloc(const std::size_t nRequestedSize) {
            // Calculate new size
            std::size_t nNewSize = m_nSize * 2;
            while (nNewSize < nRequestedSize) nNewSize *= 2;

            // Malloc new memory
            const auto pNewData = static_cast<char *>(std::realloc(this->m_pData, nNewSize * sizeof(char_type)));
            if (pNewData == nullptr) throw std::bad_alloc();

            // Swap to the new pointer
            this->m_pData = pNewData;
            this->m_nSize = nNewSize;
        }
    };

    /// Alias for basic_ofakestream with char.
    using ofakestream = basic_ofakestream<char>;
    /// Alias for basic_ofakestream with wchar_t.
    using wofakestream = basic_ofakestream<wchar_t>;
    /// Alias for basic_ofakestream with char8_t.
    using u8ofakestream = basic_ofakestream<char8_t>;
    /// Alias for basic_ofakestream with char16_t.
    using u16ofakestream = basic_ofakestream<char16_t>;
    /// Alias for basic_ofakestream with char32_t.
    using u32ofakestream = basic_ofakestream<char32_t>;
}
