/**
 * @file test.cpp
 * @brief Main file for launching the shell test collection.
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

#include <iostream>
#include <regex>

#include "BashSpark/test/test_shell.h"

using namespace bs;

/**
 * @brief Main function to launch the test series of BashSpark
 * @return Status code
 */
int main() {
    // Legal stuff
    std::cout
            << "BashSpark  Copyright (C) 2025  Dante Doménech Martínez" << std::endl
            << "This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'." << std::endl
            << "This is free software, and you are welcome to redistribute it" << std::endl
            << "under certain conditions; type `show c' for details." << std::endl;

    const auto pTest = std::make_unique<debug::test_shell>();
    pTest->test();
    return 0;
}
