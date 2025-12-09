/**
 * @file shell_node.cpp
 * @brief Implements class shell_node.
 *
 * This file contains the implementation of the `shell_node` class,
 * which provides helper tools for class `shell_parser`.
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


#include "BashSpark/shell/shell_node.h"
#include "BashSpark/tools/utf.h"

namespace bs {
    namespace {
        template<typename shell_node_type>
        std::size_t get_fpos(
            const std::vector<std::unique_ptr<shell_node_type> > &vNodes
        ) {
            static_assert(std::is_base_of_v<shell_node, shell_node_type>);
            if (vNodes.empty()) return 0;
            if (vNodes.front() == nullptr) return 0;
            return vNodes.front()->get_pos();
        }

        template<typename shell_node_type>
        std::size_t get_fpos(
            const std::unique_ptr<shell_node_type> &pNode
        ) {
            static_assert(std::is_base_of_v<shell_node, shell_node_type>);
            if (pNode == nullptr) return 0;
            return pNode->get_pos();
        }
    }

    shell_node_command_expression::shell_node_command_expression(
        std::vector<std::unique_ptr<shell_node_expandable> > &&vChildren
    )
        : shell_node(shell_node_type::SNT_COMMAND_EXPRESSION, get_fpos(vChildren)),
          shell_node_expandable(shell_node_type::SNT_COMMAND_EXPRESSION, get_fpos(vChildren)),
          m_vChildren(std::move(vChildren)) {
        if (m_vChildren.empty())
            throw shell_node_invalid_argument("Command expression takes non empty subnode group");
        if (m_vChildren[0] == nullptr)
            throw shell_node_invalid_argument("Command expression first subnode can not be nullptr");
        bool bAllNull = true;
        for (const auto &pChild: m_vChildren) {
            if (pChild != nullptr) {
                bAllNull = false;
                break;
            }
        }
        if (bAllNull) {
            throw shell_node_invalid_argument("Command expression takes no full nullptr subnode group");
        }
    }

    shell_node_str::shell_node_str(
        const shell_node_type nType,
        const std::size_t nPos,
        std::vector<std::unique_ptr<shell_node_expandable> > &&vChildren
    )
        : shell_node(nType, nPos),
          shell_node_expandable(nType, nPos),
          m_vChildren(std::move(vChildren)) {
        for (const auto &pChild: m_vChildren) {
            if (pChild == nullptr)
                throw shell_node_invalid_argument("Sentence subnode can not be null");
        }
    }

    shell_node_str_back::shell_node_str_back(
        const std::size_t nPos,
        std::unique_ptr<shell_node_evaluable> &&pCommand
    )
        : shell_node(shell_node_type::SNT_STR_BACK, nPos),
          shell_node_expandable(shell_node_type::SNT_STR_BACK, nPos),
          m_pCommand(std::move(pCommand)) {
        if (m_pCommand == nullptr)throw shell_node_invalid_argument("Sentence command can not be null");
    }

    shell_node_dollar_command::shell_node_dollar_command(
        const std::size_t nPos,
        std::unique_ptr<shell_node_evaluable> &&pCommand
    )
        : shell_node(shell_node_type::SNT_DOLLAR_COMMAND, nPos),
          shell_node_expandable(shell_node_type::SNT_DOLLAR_COMMAND, nPos),
          m_pCommand(std::move(pCommand)) {
        if (m_pCommand == nullptr)throw shell_node_invalid_argument("Sentence command can not be null");
    }

    shell_node_command::shell_node_command(
        std::unique_ptr<shell_node_command_expression> &&pCommand
    )
        : shell_node(shell_node_type::SNT_COMMAND, get_fpos(pCommand)),
          shell_node_evaluable(shell_node_type::SNT_COMMAND, get_fpos(pCommand)),
          m_pCommand(std::move(pCommand)) {
        if (m_pCommand == nullptr)throw shell_node_invalid_argument("Sentence command can not be null");
    }

    shell_node_background::shell_node_background(
        const std::size_t nPos,
        std::unique_ptr<shell_node_evaluable> &&pCommand
    )
        : shell_node(shell_node_type::SNT_BACKGROUND, nPos),
          shell_node_evaluable(shell_node_type::SNT_BACKGROUND, nPos),
          m_pCommand(std::move(pCommand)) {
        if (m_pCommand == nullptr)throw shell_node_invalid_argument("Command can not be null");
    }

    shell_node_operator::shell_node_operator(
        const shell_node_type nType,
        const std::size_t nPos,
        const int nPriority,
        std::unique_ptr<shell_node_evaluable> &&pLeft,
        std::unique_ptr<shell_node_evaluable> &&pRight
    )
        : shell_node(nType, nPos),
          shell_node_evaluable(nType, nPos),
          m_nPriority(nPriority),
          m_pLeft(std::move(pLeft)),
          m_pRight(std::move(pRight)) {
        if (pLeft == nullptr)throw shell_node_invalid_argument("Left subnode can not be null");
        if (pRight == nullptr)throw shell_node_invalid_argument("Right subnode can not be null");
    }

    shell_node_operator::shell_node_operator(
        const shell_node_type nType,
        const std::size_t nPos,
        const int nPriority
    )
        : shell_node(nType, nPos),
          shell_node_evaluable(nType, nPos),
          m_nPriority(nPriority) {
    }

    std::unique_ptr<shell_node_evaluable> shell_node_operator::make(
        const shell_node_type nType,
        const std::size_t nPos,
        std::unique_ptr<shell_node_evaluable> &&pLeft,
        std::unique_ptr<shell_node_evaluable> &&pRight
    ) {
        // Check
        if (pLeft == nullptr)throw shell_node_invalid_argument("Left subnode can not be null");
        if (pRight == nullptr)throw shell_node_invalid_argument("Right subnode can not be null");

        // Construct central
        shell_node_operator *pCentralOpp;
        switch (nType) {
            case shell_node_type::SNT_AND:
                pCentralOpp = new shell_node_and(nPos);
                break;
            case shell_node_type::SNT_OR:
                pCentralOpp = new shell_node_or(nPos);
                break;
            case shell_node_type::SNT_PIPE:
                pCentralOpp = new shell_node_pipe(nPos);
                break;
            default:
                throw shell_node_invalid_argument("Node type must be an operator");
        }

        // Central
        std::unique_ptr<shell_node_evaluable> pCentral(pCentralOpp);
        auto nCentralPriority = pCentralOpp->get_priority();

        // Get pointers
        const auto pLeftOpp = dynamic_cast<shell_node_operator *>(pLeft.get());
        const auto nLeftPriority = pLeftOpp != nullptr ? pLeftOpp->m_nPriority : 0;
        const auto pRightOpp = dynamic_cast<shell_node_operator *>(pRight.get());
        const auto nRightPriority = pRightOpp != nullptr ? pRightOpp->m_nPriority : 0;

        if (pLeftOpp == nullptr) {
            pCentralOpp->m_pLeft = std::move(pLeft);
        } else {
            if (nLeftPriority >= nCentralPriority) {
                pCentralOpp->m_pLeft = std::move(pLeft);
            } else {
                pCentralOpp->m_pLeft = std::move(pLeftOpp->m_pRight);
                pLeftOpp->m_pRight = std::move(pCentral);
                pCentral = std::move(pLeft);
                pCentralOpp = pLeftOpp;
                nCentralPriority = pCentralOpp->get_priority();
            }
        }

        if (pRightOpp == nullptr) {
            pCentralOpp->m_pRight = std::move(pRight);
        } else {
            if (nRightPriority < nCentralPriority) {
                pCentralOpp->m_pRight = std::move(pRight);
            } else {
                pCentralOpp->m_pRight = std::move(pRightOpp->m_pLeft);
                pRightOpp->m_pLeft = std::move(pCentral);
                pCentral = std::move(pRight);
            }
        }

        return pCentral;
    }

    std::unique_ptr<shell_node_evaluable> shell_node_operator::swap_left(
        std::unique_ptr<shell_node_evaluable> &&pLeft
    ) {
        if (pLeft == nullptr)throw shell_node_invalid_argument("Left subnode can not be null");
        auto pTemp = std::move(this->m_pLeft);
        this->m_pLeft = std::move(pLeft);
        return pTemp;
    }

    std::unique_ptr<shell_node_evaluable> shell_node_operator::swap_right(
        std::unique_ptr<shell_node_evaluable> &&pRight
    ) {
        if (pRight == nullptr)throw shell_node_invalid_argument("Right subnode can not be null");
        auto pTemp = std::move(this->m_pRight);
        this->m_pRight = std::move(pRight);
        return pTemp;
    }

    shell_node_test::shell_node_test(
        std::size_t nPos,
        std::unique_ptr<shell_node_expandable> &&pTest
    )
        : shell_node(shell_node_type::SNT_TEST, nPos),
          shell_node_evaluable(shell_node_type::SNT_TEST, nPos),
          m_pTest(std::move(pTest)) {
        if (m_pTest == nullptr)throw shell_node_invalid_argument("Test can not be null");
    }

    shell_node_if::shell_node_if(
        const std::size_t nPos,
        std::unique_ptr<shell_node_evaluable> &&pCondition,
        std::unique_ptr<shell_node_evaluable> &&pCaseIf,
        std::unique_ptr<shell_node_evaluable> &&pCaseElse
    )
        : shell_node(shell_node_type::SNT_IF, nPos),
          shell_node_evaluable(shell_node_type::SNT_IF, nPos),
          m_pCondition(std::move(pCondition)),
          m_pCaseIf(std::move(pCaseIf)),
          m_pCaseElse(std::move(pCaseElse)) {
        if (m_pCondition == nullptr)throw shell_node_invalid_argument("Condition can not be null");
        if (m_pCaseIf == nullptr)throw shell_node_invalid_argument("Case IF can not be null");
    }

    shell_node_for::shell_node_for(
        const std::size_t nPos,
        std::string sVariable,
        std::unique_ptr<shell_node_expandable> &&pSequence,
        std::unique_ptr<shell_node_evaluable> &&pIterative
    )
        : shell_node(shell_node_type::SNT_FOR, nPos),
          shell_node_evaluable(shell_node_type::SNT_FOR, nPos),
          m_sVariable(std::move(sVariable)),
          m_pSequence(std::move(pSequence)),
          m_pIterative(std::move(pIterative)) {
        if (m_pSequence == nullptr)throw shell_node_invalid_argument("Sequence can not be null");
        if (m_pIterative == nullptr)throw shell_node_invalid_argument("Iterative block can not be null");
    }

    shell_node_while::shell_node_while(
        const std::size_t nPos,
        std::unique_ptr<shell_node_evaluable> &&pCondition,
        std::unique_ptr<shell_node_evaluable> &&pIterative
    )
        : shell_node(shell_node_type::SNT_WHILE, nPos),
          shell_node_evaluable(shell_node_type::SNT_WHILE, nPos),
          m_pCondition(std::move(pCondition)),
          m_pIterative(std::move(pIterative)) {
        if (m_pCondition == nullptr)throw shell_node_invalid_argument("Condition can not be null");
        if (m_pIterative == nullptr)throw shell_node_invalid_argument("Iterative block can not be null");
    }

    shell_node_until::shell_node_until(
        const std::size_t nPos,
        std::unique_ptr<shell_node_evaluable> &&pCondition,
        std::unique_ptr<shell_node_evaluable> &&pIterative
    )
        : shell_node(shell_node_type::SNT_UNTIL, nPos),
          shell_node_evaluable(shell_node_type::SNT_UNTIL, nPos),
          m_pCondition(std::move(pCondition)),
          m_pIterative(std::move(pIterative)) {
        if (m_pCondition == nullptr)throw shell_node_invalid_argument("Condition can not be null");
        if (m_pIterative == nullptr)throw shell_node_invalid_argument("Iterative block can not be null");
    }

    shell_node_function::shell_node_function(
        const std::size_t nPos,
        std::unique_ptr<shell_node_expandable> &&pName,
        std::unique_ptr<shell_node_evaluable> &&pBody
    ) : shell_node(shell_node_type::SNT_FUNCTION, nPos),
        shell_node_evaluable(shell_node_type::SNT_FUNCTION, nPos),
        m_pName(std::move(pName)),
        m_pBody(std::move(pBody)) {
        if (m_pName == nullptr)throw shell_node_invalid_argument("Function name can not be null");
        if (m_pBody == nullptr)throw shell_node_invalid_argument("Function body can not be null");
    }
}
