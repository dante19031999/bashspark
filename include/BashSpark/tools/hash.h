/**
 * @file hash.h
 * @brief Implements FNV-1a hash functions for strings.
 *
 * This header file provides functions to hash strings using the FNV-1a
 * hashing algorithm. It includes overloads for C-style strings,
 * `std::string`, and `std::string_view`. The FNV-1a algorithm
 * is a simple and efficient non-cryptographic hash function.
 *
 * @note The hashing functions are constexpr where applicable, allowing
 * compile-time evaluation for constant expressions.
 *
 * @date Created on 2/11/25
 * @author Dante Doménech Martínez
 *
 * @warning The input string should be properly null-terminated for
 * C-style string functions.
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

#include <cstdint>
#include <string>
#include <string_view>

namespace bs {
    /// FVN-1 offset basis
    constexpr std::uint64_t FNV_OFFSET_BASIS = 2166136261;
    /// FVN-1 prime number
    constexpr std::uint64_t FNVPrime = 16777619;

    /**
    * @brief Hashes a string.
    *
    * This function is the base implementation of all other hashing variant funtions.
    * Currently hashes with FNV-1a.
    *
    * @param pString String to hash (C null terminated)
    * @param nLength Length of text to hash
    * @return Hash result
    */
    constexpr std::uint64_t hash(const char *const pString, const std::size_t nLength) {
        std::uint64_t nHash = FNV_OFFSET_BASIS;

        for (std::size_t i = 0; i < nLength; i++) {
            nHash ^= pString[i]; // XOR byte into the hash
            nHash *= FNVPrime; // Multiply by FNV prime
        }

        return nHash;
    }

    /**
     * @brief Hashes a string
     *
     * See \ref bs::hash(const char *pString, std::size_t nLength) "hash function implementation details"
     * for implementation details.
     *
     * @param sString String to hash
     * @return String hash
     */
    constexpr std::uint64_t hash(const std::string_view sString) {
        return hash(sString.data(), sString.length());
    }

    /**
    * @brief Hashes a string
    *
    * See \ref bs::hash(const char *pString, std::size_t nLength) "hash function implementation details"
    * for implementation details.
    *
    * @param sString String to hash
    * @return String hash
    */
    inline std::uint64_t hash(const std::string &sString) {
        return hash(sString.data(), sString.length());
    }

    /**
    * @brief Hashes a string
    *
    * See \ref bs::hash(const char *pString, std::size_t nLength) "hash function implementation details"
    * for implementation details.
    *
    * @param pString String to hash (C null terminated)
    * @return String hash
    */
    constexpr std::uint64_t hash(const char *const pString) {
        return hash(std::string_view(pString));
    }
}
