/**
 * @file shell_hash.h
 *
 * @brief Defines the `bs::shell_hash` functor implementing a 64-bit FNV-1a hash.
 *
 * This file contains the `bs::shell_hash` struct, a hashing functor compatible with
 * `std::unordered_map` and similar containers. It implements the standard
 * 64-bit FNV-1a hashing algorithm, which is simple, fast, and effective for
 * non-cryptographic use.
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

#include <cstdint>
#include <string>

namespace bs {
    /**
     * @brief Custom hash functor for strings using the 64-bit FNV-1a algorithm.
     *
     * This hash function is fast, has good distribution, and works well for
     * small and medium-sized strings such as shell tokens, variable names,
     * and keywords.
     */
    struct shell_hash {
        /// FVN offset
        static constexpr std::uint64_t FNV_OFFSET = 1469598103934665603ull;
        /// FVN prime
        static constexpr std::uint64_t FNV_PRIME = 1099511628211ull;


        /**
         * @brief Computes a 64-bit FNV-1a hash for a string.
         *
         * @param sString The input string to hash.
         * @return A 64-bit hash value.
         *
         * This implementation is based on the standard 64-bit FNV-1a algorithm:
         * ```text
         * hash = offset_basis
         * for each byte:
         *     hash ^= byte
         *     hash *= FNV_prime
         * ```
         *
         * The algorithm is simple but effective for non-cryptographic use.
         */
        std::uint64_t operator()(const std::string &sString) const noexcept {
            std::uint64_t nHash = FNV_OFFSET;
            for (const unsigned char cChar: sString) {
                nHash ^= cChar;
                nHash *= FNV_PRIME;
            }
            return nHash;
        }
    };

    /**
     * @brief Custom hash functor for strings using the 64-bit FNV-1a algorithm.
     *
     * This hash function is fast, has good distribution, and works well for
     * small and medium-sized strings such as shell tokens, variable names,
     * and keywords.
     */
    struct shell_hash_view {
        /// FVN offset
        static constexpr std::uint64_t FNV_OFFSET = 1469598103934665603ull;
        /// FVN prime
        static constexpr std::uint64_t FNV_PRIME = 1099511628211ull;


        /**
         * @brief Computes a 64-bit FNV-1a hash for a string.
         *
         * @param sString The input string to hash.
         * @return A 64-bit hash value.
         *
         * This implementation is based on the standard 64-bit FNV-1a algorithm:
         * ```text
         * hash = offset_basis
         * for each byte:
         *     hash ^= byte
         *     hash *= FNV_prime
         * ```
         *
         * The algorithm is simple but effective for non-cryptographic use.
         */
        std::uint64_t operator()(const std::string_view &sString) const noexcept {
            std::uint64_t nHash = FNV_OFFSET;
            for (const unsigned char cChar: sString) {
                nHash ^= cChar;
                nHash *= FNV_PRIME;
            }
            return nHash;
        }
    };

}
