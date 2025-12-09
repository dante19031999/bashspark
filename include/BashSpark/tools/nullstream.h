/**
 * @file nullstream.h
 * @brief Provides null stream implementations for basic input and output.
 *
 * This header file defines null input and output stream classes that
 * discard all data written to them and provide no data for input.
 *
 * The classes are designed to be used in situations where you need to
 * implement stream interfaces without actual data processing — for
 * example, during testing or when logging output with no requirement
 * to store the data.
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

#include <streambuf>
#include <ostream>
#include <istream>

namespace bs {
    /**
     * @brief A null stream buffer that ignores all input and output.
     *
     * This class provides a basic implementation of std::basic_streambuf that
     * does not store or react to any data.
     *
     * @tparam char_t The character type.
     * @tparam traits_t The traits type for the character type.
     */
    template<typename char_t, typename traits_t>
    class basic_nullbuffer : public virtual std::basic_streambuf<char_t, traits_t> {
    public:
        /// Character type of the buffer.
        using char_type = std::basic_istream<char_t, traits_t>::char_type;
        /// Character traits of the buffer.
        using traits_type = std::basic_istream<char_t, traits_t>::traits_type;
        /// Integer type representing characters.
        using int_type = std::basic_istream<char_t, traits_t>::int_type;

    public:
        /**
         * @brief Gets a number of characters from the stream.
         *
         * @param s Pointer to the buffer where characters will be stored.
         * @param n The number of characters to get.
         * @return Always returns 0 since this is a null buffer.
         */
        std::streamsize xsgetn(char_t *s, std::streamsize n) override {
            (void) s;
            (void) n;
            return 0;
        }

        /**
         * @brief Puts a number of characters into the stream.
         *
         * @param s Pointer to the buffer containing characters to put.
         * @param n The number of characters to put.
         * @return Always returns 0 since this is a null buffer.
         */
        std::streamsize xsputn(const char_t *s, std::streamsize n) override {
            (void) s;
            (void) n;
            return 0;
        }
    };

    /**
     * @brief A null input stream that provides no input.
     *
     * This class implements std::basic_istream and always returns an end-of-file state.
     *
     * @tparam char_t The character type.
     * @tparam traits_t The traits type for the character type.
     */
    template<typename char_t, typename traits_t>
    class basic_iinullstream : public virtual std::basic_istream<char_t, traits_t> {
    public:
        /// Character type of the stream.
        using char_type = std::basic_istream<char_t, traits_t>::char_type;
        /// Character traits of the stream.
        using traits_type = std::basic_istream<char_t, traits_t>::traits_type;
        /// Integer type representing characters.
        using int_type = std::basic_istream<char_t, traits_t>::int_type;
        /// Position type for the stream.
        using pos_type = std::basic_istream<char_t, traits_t>::pos_type;
        /// Offset type for the stream.
        using off_type = std::basic_istream<char_t, traits_t>::off_type;

    public:
        /**
         * @brief Default constructor.
         *
         * Initializes the null input stream with a null buffer.
         */
        basic_iinullstream()
            : std::basic_istream<char_t, traits_t>(&m_oNullBuffer) {
        }

    private:
        basic_nullbuffer<char_t, traits_t> m_oNullBuffer; /// The null buffer associated with this stream.
    };

    /**
     * @brief A null output stream that discards all output.
     *
     * This class implements std::basic_ostream and discards all characters written to it.
     *
     * @tparam char_t The character type.
     * @tparam traits_t The traits type for the character type.
     */
    template<typename char_t, typename traits_t>
    class basic_onullstream : public virtual std::basic_ostream<char_t, traits_t> {
    public:
        /// Character type of the stream.
        using char_type = std::basic_istream<char_t, traits_t>::char_type;
        /// Character traits of the stream.
        using traits_type = std::basic_istream<char_t, traits_t>::traits_type;
        /// Integer type representing characters.
        using int_type = std::basic_istream<char_t, traits_t>::int_type;
        /// Position type for the stream.
        using pos_type = std::basic_istream<char_t, traits_t>::pos_type;
        /// Offset type for the stream.
        using off_type = std::basic_istream<char_t, traits_t>::off_type;

    public:
        /**
         * @brief Default constructor.
         *
         * Initializes the null output stream with a null buffer.
         */
        basic_onullstream()
            : std::basic_ostream<char_t, traits_t>(&m_oNullBuffer) {
        }

    private:
        basic_nullbuffer<char_t, traits_t> m_oNullBuffer; /// The null buffer associated with this stream.
    };

    /**
     * @section usings Type definitions for simple streams
     */

    /// Type alias for a null output stream using char type.
    using onullstream = basic_onullstream<char, std::char_traits<char> >;
    /// Type alias for a null output stream using char8_t type.
    using u8onullstream = basic_onullstream<char8_t, std::char_traits<char8_t> >;
    /// Type alias for a null output stream using char16_t type.
    using u16onullstream = basic_onullstream<char16_t, std::char_traits<char16_t> >;
    /// Type alias for a null output stream using char32_t type.
    using u32onullstream = basic_onullstream<char32_t, std::char_traits<char32_t> >;
    /// Type alias for a null input stream using char type.
    using inullstream = basic_iinullstream<char, std::char_traits<char> >;
    /// Type alias for a null input stream using char8_t type.
    using u8inullstream = basic_iinullstream<char8_t, std::char_traits<char8_t> >;
    /// Type alias for a null input stream using char16_t type.
    using u16inullstream = basic_iinullstream<char16_t, std::char_traits<char16_t> >;
    /// Type alias for a null input stream using char32_t type.
    using u32inullstream = basic_iinullstream<char32_t, std::char_traits<char32_t> >;
} // namespace d
