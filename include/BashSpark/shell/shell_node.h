/**
 * @file shell_node.h
 * @brief Defines the shell node hierarchy used by the parser.
 *
 * This file provides the node types and helper node classes used by the
 * shell parser. Nodes are categorized as *expandable* (produce string tokens)
 * or *evaluable* (execute behavior and return a shell_status).
 *
 * Medium verbosity Doxygen comments: class/constructor/method brief,
 * parameters, return values and exceptions where appropriate.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "BashSpark/shell/shell_status.h"

namespace bs {
    class shell_session;
}

namespace bs {
    /**
     * @enum shell_node_type
     * @brief Types of nodes recognized by the shell parser.
     *
     * Values include language constructs (IF/FOR/WHILE), tokens (WORD, ARG),
     * expansion tokens ($VAR, command substitution) and executable nodes
     * (COMMAND, COMMAND_BLOCK, etc.).
     */
    enum class shell_node_type {
        SNT_COMMAND_EXPRESSION,
        SNT_STR_SIMPLE,
        SNT_STR_DOUBLE,
        SNT_STR_BACK,
        SNT_WORD,
        SNT_UNICODE,
        SNT_ARG,
        SNT_VARIABLE,
        SNT_DOLLAR_SPECIAL,
        SNT_DOLLAR_VARIABLE,
        SNT_DOLLAR_VARIABLE_DHOP,
        SNT_DOLLAR_ARG,
        SNT_DOLLAR_ARG_DHOP,
        SNT_DOLLAR_COMMAND,
        SNT_BACKGROUND,
        SNT_AND,
        SNT_PIPE,
        SNT_OR,
        // Structure
        SNT_IF,
        SNT_TEST,
        SNT_FOR,
        SNT_WHILE,
        SNT_UNTIL,
        SNT_BREAK,
        SNT_CONTINUE,
        SNT_FUNCTION,
        // Executable
        SNT_NULL_COMMAND,
        SNT_COMMAND,
        SNT_COMMAND_BLOCK,
        SNT_COMMAND_BLOCK_SUBSHELL,
    };

    /**
     * @class shell_node_invalid_argument
     * @brief Exception thrown when a node is constructed with invalid args.
     *
     * Thin wrapper around std::invalid_argument used to indicate errors
     * detected while building node objects.
     */
    class shell_node_invalid_argument final : public std::invalid_argument {
    public:
        /**
         * @brief Construct the exception with a descriptive message.
         * @param sMessage Description of the error.
         */
        explicit shell_node_invalid_argument(const std::string &sMessage)
            : std::invalid_argument(sMessage) {
        }
    };

    /**
     * @class shell_node
     * @brief Base class for all parser nodes.
     *
     * Holds the node type and its position in the source stream. It is an abstract
     * base for both expandable and evaluable node kinds.
     */
    class shell_node {
    public:
        /// Virtual default destructor for polymorphic deletion.
        virtual ~shell_node() = default;

    protected:
        /**
         * @brief Protected constructor used by derived classes.
         * @param nType Node type.
         * @param nPos Position in source stream (character offset).
         */
        shell_node(
            const shell_node_type nType,
            const std::size_t nPos
        )
            : m_nType(nType),
              m_nPos(nPos) {
        }

    public:
        /**
         * @brief Get the node type.
         * @return shell_node_type The node's type.
         */
        [[nodiscard]] shell_node_type get_type() const noexcept {
            return this->m_nType;
        }

        /**
         * @brief Get node position in the original input stream.
         * @return std::size_t Character offset where node was created.
         */
        [[nodiscard]] std::size_t get_pos() const noexcept {
            return this->m_nPos;
        }

    protected:
        const shell_node_type m_nType; ///< Node type.
        const std::size_t m_nPos; ///< Position in the input stream.
    };

    /**
     * @class shell_node_evaluable
     * @brief Base interface for nodes that can be evaluated (executed).
     *
     * Evaluatable nodes perform actions and return a `shell_status` indicating
     * the result of execution.
     */
    class shell_node_evaluable : public virtual shell_node {
    public:
        /**
         * @brief Construct an evaluable node.
         * @param nType Node type.
         * @param nPos Node position.
         */
        shell_node_evaluable(
            const shell_node_type nType,
            const std::size_t nPos
        )
            : shell_node(nType, nPos) {
        }

    public:
        /**
         * @brief Evaluate (execute) the node.
         * @param oSession Session context used during evaluation.
         * @return shell_status Result status of the execution.
         */
        virtual shell_status evaluate(shell_session &oSession) const =0;
    };

    /**
     * @class shell_node_expandable
     * @brief Base interface for nodes that expand to tokens (strings).
     *
     * Expandable nodes append one or more strings into the `vTokens` vector.
     * The `bSplit` flag indicates whether word-splitting rules should be applied.
     */
    class shell_node_expandable : public virtual shell_node {
    public:
        /**
         * @brief Construct an expandable node.
         * @param nType Node type.
         * @param nPos Node position.
         */
        shell_node_expandable(
            const shell_node_type nType,
            const std::size_t nPos
        )
            : shell_node(nType, nPos) {
        }

    public:
        /**
         * @brief Expand the node into textual tokens.
         * @param vTokens Vector where generated token strings will be appended.
         * @param oSession Session context (used for variable/argument lookup, etc.).
         * @param bSplit When true apply word-splitting semantics; otherwise keep as single token.
         */
        virtual void expand(
            std::vector<std::string> &vTokens,
            shell_session &oSession,
            bool bSplit
        ) const =0;
    };

    /**
     * @class shell_node_command_expression
     * @brief Represents a command expression composed of expandable children.
     *
     * The node holds ownership of a vector of expandable child nodes and expands by
     * expanding each child in sequence.
     */
    class shell_node_command_expression final : public shell_node_expandable {
    public:
        /**
         * @brief Construct a command expression node.
         * @throw shell_node_invalid_argument If vChildren is empty.
         * @throw shell_node_invalid_argument If first vChildren is null pointer.
         * @throw shell_node_invalid_argument If all vChildren are null pointers.
         * @param vChildren Vector of child expandable nodes (moved in).
         */
        explicit shell_node_command_expression(
            std::vector<std::unique_ptr<shell_node_expandable> > &&vChildren
        );

    public:
        /**
         * @brief Expand the expression by expanding its children in order.
         * @inheritdoc
         */
        void expand(
            std::vector<std::string> &vTokens,
            shell_session &oSession,
            bool bSplit
        ) const override;

    public:
        /**
         * @brief Access the child nodes.
         * @return const reference to the internal children vector.
         */
        [[nodiscard]] const std::vector<std::unique_ptr<shell_node_expandable> > &get_children() const noexcept {
            return this->m_vChildren;
        }

    private:
        std::vector<std::unique_ptr<shell_node_expandable> > m_vChildren; ///< Child expandables.
    };

    /**
     * @class shell_node_str
     * @brief Base for string-like nodes composed of expandable fragments.
     *
     * Used by quoted and unquoted string types. Holds child expandables which
     * are concatenated during expansion.
     */
    class shell_node_str : public shell_node_expandable {
    protected:
        /**
         * @brief Construct a string node.
         * @throw shell_node_invalid_argument If vChildren contains null pointer.
         * @param nType Specific string node type.
         * @param nPos Position in the input stream.
         * @param vChildren Child expandable fragments.
         */
        shell_node_str(
            shell_node_type nType,
            std::size_t nPos,
            std::vector<std::unique_ptr<shell_node_expandable> > &&vChildren
        );

    public:
        /**
         * @brief Expand by concatenating child fragments into one token.
         * @inheritdoc
         */
        void expand(
            std::vector<std::string> &vTokens,
            shell_session &oSession,
            bool bSplit
        ) const override;

        /**
         * @brief Get the children fragments.
         * @return const reference to children vector.
         */
        [[nodiscard]] const std::vector<std::unique_ptr<shell_node_expandable> > &get_children() const noexcept {
            return this->m_vChildren;
        }

    private:
        std::vector<std::unique_ptr<shell_node_expandable> > m_vChildren; ///< Child fragments.
    };

    /**
     * @class shell_node_str_simple
     * @brief Simple (unquoted) string node.
     *
     * Inherits behavior from shell_node_str and identifies as SNT_STR_SIMPLE.
     */
    class shell_node_str_simple final : public shell_node_str {
    public:
        /**
         * @brief Construct an unquoted string node.
         * @param nPos Position in stream.
         * @param vChildren Child fragments.
         */
        shell_node_str_simple(
            const std::size_t nPos,
            std::vector<std::unique_ptr<shell_node_expandable> > &&vChildren
        )
            : shell_node(shell_node_type::SNT_STR_SIMPLE, nPos),
              shell_node_str(shell_node_type::SNT_STR_SIMPLE, nPos, std::move(vChildren)) {
        }
    };

    /**
     * @class shell_node_str_double
     * @brief Double-quoted string node.
     *
     * Behaves like shell_node_str but represents double-quoted input (SNT_STR_DOUBLE).
     */
    class shell_node_str_double final : public shell_node_str {
    public:
        /**
         * @brief Construct a double-quoted string node.
         * @param nPos Position in stream.
         * @param vChildren Child fragments.
         */
        shell_node_str_double(
            const std::size_t nPos,
            std::vector<std::unique_ptr<shell_node_expandable> > &&vChildren
        )
            : shell_node(shell_node_type::SNT_STR_DOUBLE, nPos),
              shell_node_str(shell_node_type::SNT_STR_DOUBLE, nPos, std::move(vChildren)) {
        }
    };

    /**
     * @class shell_node_str_back
     * @brief Command-substitution string node (backticks or $(...)).
     *
     * Holds an evaluable subcommand which is executed during expansion and its
     * output used as the string fragment.
     */
    class shell_node_str_back final : public shell_node_expandable {
    public:
        /**
         * @brief Construct a command-substitution node.
         * @throw shell_node_invalid_argument If pCommand is null.
         * @param nPos Position of the opening quote in the stream.
         * @param pCommand Subcommand to evaluate when expanding.
         */
        shell_node_str_back(
            std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pCommand
        );

    public:
        /**
         * @brief Expand by executing the subcommand and inserting its output.
         * @inheritdoc
         */
        void expand(
            std::vector<std::string> &vTokens,
            shell_session &oSession,
            bool bSplit
        ) const override;

        /**
         * @brief Get the underlying evaluable subcommand.
         * @return pointer to the evaluable node (non-owning).
         */
        [[nodiscard]] const shell_node_evaluable *get_command() const noexcept {
            return this->m_pCommand.get();
        }

    private:
        std::unique_ptr<shell_node_evaluable> m_pCommand; ///< Owned subcommand.
    };

    /**
     * @class shell_node_word
     * @brief A plain word token node.
     *
     * Contains raw text that will be appended as-is when expanded.
     */
    class shell_node_word final : public shell_node_expandable {
    public:
        /**
         * @brief Construct a word node.
         * @param nPos Position in stream.
         * @param sText The literal word text (moved in).
         */
        shell_node_word(
            const std::size_t nPos,
            std::string sText
        ) : shell_node(shell_node_type::SNT_WORD, nPos),
            shell_node_expandable(shell_node_type::SNT_WORD, nPos),
            m_sText(std::move(sText)) {
        }

    public:
        /**
         * @brief Expand by appending the literal text as a token.
         * @inheritdoc
         */
        void expand(
            std::vector<std::string> &vTokens,
            shell_session &oSession,
            bool bSplit
        ) const override;

    public:
        /**
         * @brief Get the stored text.
         * @return std::string Literal word content.
         */
        [[nodiscard]] std::string get_text() const noexcept {
            return this->m_sText;
        }

    private:
        std::string m_sText; ///< Literal content.
    };

    /**
     * @class shell_node_unicode
     * @brief A single Unicode codepoint node.
     *
     * Represents a single UTF-32 codepoint that expands into the corresponding
     * UTF-8 encoded character(s).
     */
    class shell_node_unicode final : public shell_node_expandable {
    public:
        /**
         * @brief Construct a unicode codepoint node.
         * @param nPos Position in input.
         * @param cCharacter UTF-32 codepoint value.
         */
        shell_node_unicode(
            const std::size_t nPos,
            const char32_t cCharacter
        ) : shell_node(shell_node_type::SNT_UNICODE, nPos),
            shell_node_expandable(shell_node_type::SNT_UNICODE, nPos),
            m_cCharacter(cCharacter) {
        }

    public:
        /**
         * @brief Expand by converting the codepoint into UTF-8 and appending it.
         */
        void expand(
            std::vector<std::string> &vTokens,
            shell_session &oSession,
            bool bSplit
        ) const override;

    public:
        /**
         * @brief Returns the stored UTF-32 codepoint.
         * @return char32_t The codepoint.
         */
        [[nodiscard]] char32_t get_character() const noexcept {
            return this->m_cCharacter;
        }

    private:
        char32_t m_cCharacter; ///< Codepoint value.
    };

    /**
     * @class shell_node_session_extractor
     * @brief Base for nodes that extract values from the session (args/vars/etc.).
     *
     * Derived classes implement get_value() to read a value from the provided session.
     * During expand(), the extracted value is appended to the token list.
     */
    class shell_node_session_extractor : public shell_node_expandable {
    public:
        /**
         * @brief Construct a session extractor node.
         * @param nType Specific node type.
         * @param nPos Position in input.
         */
        shell_node_session_extractor(
            const shell_node_type nType,
            const std::size_t nPos
        )
            : shell_node(nType, nPos),
              shell_node_expandable(nType, nPos) {
        }

    public:
        /**
         * @brief Expand by retrieving the value via get_value() and appending it.
         * @param vTokens Token vector to update.
         * @param oSession Active shell session.
         * @param bSplit Whether to apply word-splitting rules.
         */
        void expand(
            std::vector<std::string> &vTokens,
            shell_session &oSession,
            bool bSplit
        ) const override;

    public:
        /**
         * @brief Retrieve the textual value from the given session.
         * @param oSession The session to query.
         * @return std::string The extracted string value.
         */
        [[nodiscard]] virtual std::string get_value(const shell_session &oSession) const =0;
    };

    /**
     * @class shell_node_arg
     * @brief Extract positional argument from the session (e.g. $1, $2).
     */
    class shell_node_arg final : public shell_node_session_extractor {
    public:
        /**
         * @brief Construct an argument extractor.
         * @param nPos Position in stream.
         * @param nArg Argument index to extract.
         */
        shell_node_arg(
            const std::size_t nPos,
            const std::uint64_t nArg
        ) : shell_node(shell_node_type::SNT_ARG, nPos),
            shell_node_session_extractor(shell_node_type::SNT_ARG, nPos),
            m_nArg(nArg) {
        }

    public:
        /**
         * @brief Return the argument value from the session.
         * @param oSession Session that holds arguments.
         * @return std::string Argument string (empty if out of range).
         */
        [[nodiscard]] std::string get_value(const shell_session &oSession) const override;

        /**
         * @brief Get the argument index this node references.
         * @return std::size_t Argument index.
         */
        [[nodiscard]] std::uint64_t get_arg() const noexcept {
            return this->m_nArg;
        }

    private:
        std::uint64_t m_nArg; ///< Index of the argument.
    };

    /**
     * @class shell_node_variable
     * @brief Extract a named shell variable from the session environment.
     *
     * This node reads a variable by name when expanded.
     */
    class shell_node_variable final : public shell_node_session_extractor {
    public:
        /**
         * @brief Construct a variable extractor.
         * @param nPos Position in stream.
         * @param sVariable Name of the variable (moved in).
         */
        shell_node_variable(
            const std::size_t nPos,
            std::string sVariable
        ) : shell_node(shell_node_type::SNT_VARIABLE, nPos),
            shell_node_session_extractor(shell_node_type::SNT_VARIABLE, nPos),
            m_sVariable(std::move(sVariable)) {
        }

    public:
        /**
         * @brief Get the variable's value from the session.
         * @param oSession Session providing environment variables.
         * @return std::string Value (empty if not present).
         */
        [[nodiscard]] std::string get_value(const shell_session &oSession) const override;

        /**
         * @brief Name of the referenced variable.
         * @return std::string Variable name.
         */
        [[nodiscard]] std::string get_variable() const noexcept {
            return this->m_sVariable;
        }

    private:
        std::string m_sVariable; ///< Variable name.
    };

    /**
     * @class shell_node_dollar_variable
     * @brief Dollar-prefixed variable node (may have different semantics).
     *
     * Semantics may differ from shell_node_variable (implementation detail).
     */
    class shell_node_dollar_variable final : public shell_node_session_extractor {
    public:
        /**
         * @brief Construct a dollar-variable extractor.
         * @param nPos Position in stream.
         * @param sVariable Variable name.
         */
        shell_node_dollar_variable(
            const std::size_t nPos,
            std::string sVariable
        ) : shell_node(shell_node_type::SNT_DOLLAR_VARIABLE, nPos),
            shell_node_session_extractor(shell_node_type::SNT_DOLLAR_VARIABLE, nPos),
            m_sVariable(std::move(sVariable)) {
        }

    public:
        /**
         * @brief Get the value for this dollar-variable from the session.
         * @param oSession Session context.
         * @return std::string Extracted value.
         */
        [[nodiscard]] std::string get_value(const shell_session &oSession) const override;

        /**
         * @brief Return the referenced variable name.
         * @return std::string Variable name.
         */
        [[nodiscard]] std::string get_variable() const noexcept {
            return this->m_sVariable;
        }

    private:
        std::string m_sVariable; ///< Variable name.
    };

    /**
     * @class shell_node_dollar_arg
     * @brief Extract an argument referenced with a leading $ (e.g. $@, $* or $1).
     */
    class shell_node_dollar_arg final : public shell_node_session_extractor {
    public:
        /**
         * @brief Construct a dollar-argument extractor.
         * @param nPos Position in stream.
         * @param nArg Argument index referenced.
         */
        shell_node_dollar_arg(
            const std::size_t nPos,
            const std::uint64_t nArg
        ) : shell_node(shell_node_type::SNT_DOLLAR_ARG, nPos),
            shell_node_session_extractor(shell_node_type::SNT_DOLLAR_ARG, nPos),
            m_nArg(nArg) {
        }

    public:
        /**
         * @brief Retrieve the argument value via the session.
         * @param oSession Active session.
         * @return std::string Argument string.
         */
        [[nodiscard]] std::string get_value(const shell_session &oSession) const override;

        /**
         * @brief Get referenced argument index.
         * @return std::size_t Argument index.
         */
        [[nodiscard]] std::uint64_t get_arg() const noexcept {
            return this->m_nArg;
        }

    private:
        std::uint64_t m_nArg; ///< Argument index.
    };

    /**
     * @class shell_node_dollar_arg_dhop
     * @brief Dollar-argument with "double-hop" semantics (implementation-specific).
     */
    class shell_node_dollar_arg_dhop final : public shell_node_session_extractor {
    public:
        /**
         * @brief Construct a double-hop dollar argument node.
         * @param nPos Position in stream.
         * @param nArg Argument index.
         */
        shell_node_dollar_arg_dhop(
            const std::size_t nPos,
            const std::uint64_t nArg
        ) : shell_node(shell_node_type::SNT_DOLLAR_ARG_DHOP, nPos),
            shell_node_session_extractor(shell_node_type::SNT_DOLLAR_ARG_DHOP, nPos),
            m_nArg(nArg) {
        }

    public:
        /**
         * @brief Get the value for this double-hop argument.
         * @param oSession Session to query.
         * @return std::string Extracted value.
         */
        [[nodiscard]] std::string get_value(const shell_session &oSession) const override;

        /**
         * @brief Get the argument index.
         * @return std::size_t Index value.
         */
        [[nodiscard]] std::uint64_t get_arg() const noexcept {
            return this->m_nArg;
        }

    private:
        std::uint64_t m_nArg; ///< Argument index.
    };

    /**
     * @class shell_node_dollar_variable_dhop
     * @brief Dollar-variable with double-hop lookup semantics.
     */
    class shell_node_dollar_variable_dhop final : public shell_node_session_extractor {
    public:
        /**
         * @brief Construct a double-hop dollar variable node.
         * @param nPos Position in stream.
         * @param sVariable Variable name.
         */
        shell_node_dollar_variable_dhop(
            const std::size_t nPos,
            std::string sVariable
        ) : shell_node(shell_node_type::SNT_DOLLAR_VARIABLE_DHOP, nPos),
            shell_node_session_extractor(shell_node_type::SNT_DOLLAR_VARIABLE_DHOP, nPos),
            m_sVariable(std::move(sVariable)) {
        }

    public:
        /**
         * @brief Retrieve the variable's double-hop value.
         * @param oSession Session context.
         * @return std::string Extracted value.
         */
        [[nodiscard]] std::string get_value(const shell_session &oSession) const override;

        /**
         * @brief Get the referenced variable name.
         * @return std::string Variable name.
         */
        [[nodiscard]] std::string get_variable() const noexcept {
            return this->m_sVariable;
        }

    private:
        std::string m_sVariable; ///< Variable name.
    };

    /**
     * @class shell_node_dollar_command
     * @brief Command-substitution node used in $() or backticks when appearing inside other contexts.
     *
     * The node owns an evaluable subcommand that will be executed during expansion.
     */
    class shell_node_dollar_command final : public shell_node_expandable {
    public:
        /**
          * @brief Construct a dollar-command node.
          * @throw shell_node_invalid_argument If pCommand is null.
          * @param nPos Command position in the stream.
          * @param pCommand Owned evaluable subcommand.
          */
        shell_node_dollar_command(
            std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pCommand
        );

    public:
        /**
         * @brief Expand by executing the subcommand and inserting its textual output.
         * @inheritdoc
         */
        void expand(
            std::vector<std::string> &vTokens,
            shell_session &oSession,
            bool bSplit
        ) const override;

        /**
         * @brief Get non-owning pointer to the subcommand.
         * @return const shell_node_evaluable* Pointer to the subcommand.
         */
        [[nodiscard]] const shell_node_evaluable *get_command() const noexcept {
            return this->m_pCommand.get();
        }

    private:
        std::unique_ptr<shell_node_evaluable> m_pCommand; ///< Owned evaluable subcommand.
    };

    /**
     * @class shell_node_dollar_special
     * @brief Special dollar items such as $?, $#, $$, etc. (implementation-specific).
     */
    class shell_node_dollar_special final : public shell_node_session_extractor {
    public:
        /**
         * @brief Construct a dollar-special node.
         * @param nPos Position in stream.
         * @param cItem Character identifying the special item.
         */
        shell_node_dollar_special(
            const std::size_t nPos,
            const char cItem
        )
            : shell_node(shell_node_type::SNT_DOLLAR_SPECIAL, nPos),
              shell_node_session_extractor(shell_node_type::SNT_DOLLAR_SPECIAL, nPos),
              m_cItem(cItem) {
        }

    public:
        /**
         * @brief Return the special item's textual value.
         * @param oSession Active session for context.
         * @return std::string Resolved value for the special item.
         */
        [[nodiscard]] std::string get_value(const shell_session &oSession) const override;

        /**
         * @brief Get the identifying character for this special item.
         * @return char Character such as '?', '#', etc.
         */
        [[nodiscard]] char get_item() const noexcept {
            return this->m_cItem;
        }

    private:
        char m_cItem; ///< Special item code.
    };

    /**
     * @class shell_node_null_command
     * @brief Represents a no-op command node (useful as placeholder).
     */
    class shell_node_null_command final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct a null command node.
         * @param nPos Position in stream.
         */
        explicit shell_node_null_command(
            const std::size_t nPos
        )
            : shell_node(shell_node_type::SNT_NULL_COMMAND, nPos),
              shell_node_evaluable(shell_node_type::SNT_NULL_COMMAND, nPos) {
        }

    public:
        /**
         * @brief Evaluate the null command (typically returns success).
         * @param oSession Session context.
         * @return shell_status Execution result.
         */
        shell_status evaluate(shell_session &oSession) const override;
    };

    /**
     * @class shell_node_command
     * @brief Wraps a command expression and executes it as a command.
     *
     * The command expression is an expandable node that will be expanded into
     * argument tokens and then executed according to shell semantics.
     */
    class shell_node_command final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct a command node.
         * @throw shell_node_invalid_argument If expression is null.
         * @param pCommand Owned command expression.
         */
        explicit shell_node_command(
            std::unique_ptr<shell_node_command_expression> &&pCommand
        );

    public:
        /**
         * @brief Evaluate (execute) the command expression.
         * @param oSession Session context.
         * @return shell_status Result of command execution.
         */
        shell_status evaluate(shell_session &oSession) const override;

        /**
         * @brief Get pointer to the underlying command expression.
         * @return  Non-owning pointer.
         */
        [[nodiscard]] const shell_node_command_expression *get_command() const noexcept {
            return this->m_pCommand.get();
        }

    private:
        std::unique_ptr<shell_node_command_expression> m_pCommand; ///< Owned expression.
    };

    /**
     * @class shell_node_command_block
     * @brief A sequence of evaluable nodes executed in order (a block).
     */
    class shell_node_command_block final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct a command block.
         * @param nPos Position in stream.
         * @param vSubCommands Vector of evaluable subcommands (moved in).
         */
        shell_node_command_block(
            const std::size_t nPos,
            std::vector<std::unique_ptr<shell_node_evaluable> > &&vSubCommands
        )
            : shell_node(shell_node_type::SNT_COMMAND_BLOCK, nPos),
              shell_node_evaluable(shell_node_type::SNT_COMMAND_BLOCK, nPos),
              m_vSubCommands(std::move(vSubCommands)) {
        }

    public:
        /**
         * @brief Evaluate each subcommand in order.
         * @param oSession Session context.
         * @return shell_status Last subcommand's status or overall status.
         */
        shell_status evaluate(shell_session &oSession) const override;

        /**
         * @brief Access subcommands.
         * @return const reference to vector of subcommands.
         */
        [[nodiscard]] const std::vector<std::unique_ptr<shell_node_evaluable> > &get_children() const noexcept {
            return this->m_vSubCommands;
        }

    private:
        std::vector<std::unique_ptr<shell_node_evaluable> > m_vSubCommands; ///< Children.
    };

    /**
     * @class shell_node_command_block_subshell
     * @brief Command block executed in a subshell (may isolate environment changes).
     */
    class shell_node_command_block_subshell final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct a subshell command block.
         * @param nPos Position in stream.
         * @param vSubCommands Subcommands to run in the subshell.
         */
        shell_node_command_block_subshell(
            const std::size_t nPos,
            std::vector<std::unique_ptr<shell_node_evaluable> > &&vSubCommands
        )
            : shell_node(shell_node_type::SNT_COMMAND_BLOCK_SUBSHELL, nPos),
              shell_node_evaluable(shell_node_type::SNT_COMMAND_BLOCK_SUBSHELL, nPos),
              m_vSubCommands(std::move(vSubCommands)) {
        }

    public:
        /**
         * @brief Evaluate the block in a subshell context.
         * @param oSession Session context.
         * @return shell_status Status from the subshell execution.
         */
        shell_status evaluate(shell_session &oSession) const override;

        /**
         * @brief Access the subshell's subcommands.
         * @return const reference to vector of subcommands.
         */
        [[nodiscard]] const std::vector<std::unique_ptr<shell_node_evaluable> > &get_children() const noexcept {
            return this->m_vSubCommands;
        }

    private:
        std::vector<std::unique_ptr<shell_node_evaluable> > m_vSubCommands; ///< Children.
    };

    /**
     * @class shell_node_background
     * @brief Evaluates a subcommand in background semantics (implementation-specific).
     *
     * The constructor takes ownership of the subcommand to be run in background.
     */
    class shell_node_background final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct a background command node.
         * @throw shell_node_invalid_argument If pCommand is null.
         * @param nPos Background operator position.
         * @param pCommand Owned evaluable subcommand.
         */
        shell_node_background(
            std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pCommand
        );

    public:
        /**
         * @brief Evaluate the background node (typically launches and returns immediately).
         * @param oSession Session context.
         * @return shell_status Execution result or job control status.
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
         * @brief Get pointer to the underlying command.
         * @return  Non-owning pointer to underlying command.
         */
        [[nodiscard]] const shell_node_evaluable *get_command() const noexcept {
            return this->m_pCommand.get();
        }

    private:
        std::unique_ptr<shell_node_evaluable> m_pCommand; ///< Owned command.
    };

    /**
     * @class shell_node_operator
     * @brief Base class for binary operator nodes (pipe, and, or) with priority handling.
     *
     * Operator nodes hold left and right evaluable children and a priority used when
     * building or reorganizing operator trees.
     */
    class shell_node_operator : public shell_node_evaluable {
    public:
        /// Priority for operator pipe
        constexpr static int PRIORITY_PIPE = 5;
        /// Priority for operator and
        constexpr static int PRIORITY_AND = 4;
        /// Priority for operator or
        constexpr static int PRIORITY_OR = 3;

    public:
        /**
         * @brief Makes an operator node.
         *
         * Utility that constructs an appropriate operator subclass and performs
         * priority handling if left/right are operator nodes too.
         *
         * @throw shell_node_invalid_argument If pLeft or pRight is null.
         * @throw shell_node_invalid_argument If nType is not an operator node type.
         * @param nType Operator node type.
         * @param nPos Node position in stream.
         * @param pLeft Left operand (moved in).
         * @param pRight Right operand (moved in).
         * @return std::unique_ptr<shell_node_evaluable> Newly created operator node.
         */
        static std::unique_ptr<shell_node_evaluable> make(
            shell_node_type nType,
            std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pLeft,
            std::unique_ptr<shell_node_evaluable> &&pRight
        );

    protected:
        /**
         * @brief Construct an operator with both operands.
         * @throw shell_node_invalid_argument If pLeft or pRight is null.
         * @param nType Node type (operator).
         * @param nPos Position in stream.
         * @param nPriority Operator priority.
         * @param pLeft Left operand.
         * @param pRight Right operand.
         */
        shell_node_operator(
            shell_node_type nType,
            std::size_t nPos,
            int nPriority,
            std::unique_ptr<shell_node_evaluable> &&pLeft,
            std::unique_ptr<shell_node_evaluable> &&pRight
        );

    protected:
        /**
         * @warning This constructor supports constructing an operator node without
         * left/right operands. Use with caution; the node will be invalid until
         * left and right are set.
         *
         * @param nType Node type (must be operator).
         * @param nPos Position in stream.
         * @param nPriority Priority of the operator.
         */
        shell_node_operator(
            shell_node_type nType,
            std::size_t nPos,
            int nPriority
        );

    public:
        /**
         * @brief Get the operator priority.
         * @return int Priority value (higher means evaluated earlier).
         */
        [[nodiscard]] int get_priority() const noexcept {
            return this->m_nPriority;
        }

        /**
         * @brief Get (non-owning) pointer to the left operand.
         * @return const shell_node_evaluable* Left operand or nullptr.
         */
        [[nodiscard]] const shell_node_evaluable *get_left() const noexcept {
            return this->m_pLeft.get();
        }

        /**
         * @brief Get (non-owning) pointer to the right operand.
         * @return const shell_node_evaluable* Right operand or nullptr.
         */
        [[nodiscard]] const shell_node_evaluable *get_right() const noexcept {
            return this->m_pRight.get();
        }

        /**
         * @brief Swap-in a new left operand, returning the old one.
         * @throw shell_node_invalid_argument If pLeft is null.
         * @param pLeft New left operand (moved in).
         * @return std::unique_ptr<shell_node_evaluable> The previous left operand.
         */
        std::unique_ptr<shell_node_evaluable> swap_left(std::unique_ptr<shell_node_evaluable> &&pLeft);

        /**
         * @brief Swap-in a new right operand, returning the old one.
         * @throw shell_node_invalid_argument If pRight is null.
         * @param pRight New right operand (moved in).
         * @return std::unique_ptr<shell_node_evaluable> The previous right operand.
         */
        std::unique_ptr<shell_node_evaluable> swap_right(std::unique_ptr<shell_node_evaluable> &&pRight);

    private:
        int m_nPriority;
        std::unique_ptr<shell_node_evaluable> m_pLeft;
        std::unique_ptr<shell_node_evaluable> m_pRight;
    };

    /**
     * @class shell_node_and
     * @brief Logical AND operator node (executes right only if left succeeded).
     */
    class shell_node_and final : public shell_node_operator {
        friend class shell_node_operator;

    public:
        /**
         * @brief Construct an AND node.
         * @throw shell_node_invalid_argument If pLeft or pRight is null.
         * @param nPos Position in stream.
         * @param pLeft Left operand.
         * @param pRight Right operand.
         */
        shell_node_and(
            const std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pLeft,
            std::unique_ptr<shell_node_evaluable> &&pRight
        )
            : shell_node(shell_node_type::SNT_AND, nPos),
              shell_node_operator(
                  shell_node_type::SNT_AND, nPos, PRIORITY_AND,
                  std::move(pLeft), std::move(pRight)
              ) {
        }

    public:
        /**
         * @brief Evaluate logical AND semantics.
         * @param oSession Session context.
         * @return shell_status Last command result according to AND semantics.
         */
        shell_status evaluate(shell_session &oSession) const override;

    private:
        explicit shell_node_and(
            const std::size_t nPos
        )
            : shell_node(shell_node_type::SNT_AND, nPos),
              shell_node_operator(shell_node_type::SNT_AND, nPos, PRIORITY_AND) {
        }
    };

    /**
     * @class shell_node_pipe
     * @brief Pipe operator node (connects stdout of left to stdin of right).
     */
    class shell_node_pipe final : public shell_node_operator {
        friend class shell_node_operator;

    public:
        /**
         * @brief Construct a pipe node.
         * @throw shell_node_invalid_argument If pLeft or pRight is null.
         * @param nPos Position in stream.
         * @param pLeft Left operand (producer).
         * @param pRight Right operand (consumer).
         */
        shell_node_pipe(
            const std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pLeft,
            std::unique_ptr<shell_node_evaluable> &&pRight
        )
            : shell_node(shell_node_type::SNT_PIPE, nPos),
              shell_node_operator(
                  shell_node_type::SNT_PIPE, nPos, PRIORITY_PIPE,
                  std::move(pLeft), std::move(pRight)
              ) {
        }

    public:
        /**
         * @brief Evaluate pipe semantics (setup streams and execute both sides).
         * @param oSession Session context.
         * @return shell_status Result of the pipeline or last command status.
         */
        shell_status evaluate(shell_session &oSession) const override;

    private:
        explicit shell_node_pipe(
            const std::size_t nPos
        )
            : shell_node(shell_node_type::SNT_PIPE, nPos),
              shell_node_operator(shell_node_type::SNT_PIPE, nPos, PRIORITY_PIPE) {
        }
    };

    /**
     * @class shell_node_or
     * @brief Logical OR operator node (executes right only if left failed).
     */
    class shell_node_or final : public shell_node_operator {
        friend class shell_node_operator;

    public:
        /**
         * @brief Construct an OR node.
         * @throw shell_node_invalid_argument If pLeft or pRight is null.
         * @param nPos Position in stream.
         * @param pLeft Left operand.
         * @param pRight Right operand.
         */
        shell_node_or(
            const std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pLeft,
            std::unique_ptr<shell_node_evaluable> &&pRight
        )
            : shell_node(shell_node_type::SNT_OR, nPos),
              shell_node_operator(
                  shell_node_type::SNT_OR, nPos, PRIORITY_OR,
                  std::move(pLeft), std::move(pRight)
              ) {
        }

    public:
        /**
         * @brief Evaluate logical OR semantics.
         * @param oSession Session context.
         * @return shell_status Result according to OR semantics.
         */
        shell_status evaluate(shell_session &oSession) const override;

    private:
        explicit shell_node_or(
            const std::size_t nPos
        )
            : shell_node(shell_node_type::SNT_OR, nPos),
              shell_node_operator(shell_node_type::SNT_OR, nPos, PRIORITY_OR) {
        }
    };

    /**
   * @class shell_node_test
   * @brief Node that evaluates an expandable expression as a test/condition.
   *
   * This node wraps an expandable node used as a boolean-style test. The wrapped
   * expandable will be expanded to tokens and then interpreted according to the
   * shell's test semantics during evaluation.
   *
   */
    class shell_node_test final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct a test node.
         * @throw shell_node_invalid_argument If \p pTest is null.
         * @param nPos Position in the input stream where the test begins.
         * @param pTest Owned expandable node representing the test expression.
         */
        shell_node_test(
            std::size_t nPos,
            std::unique_ptr<shell_node_expandable> &&pTest
        );

    public:
        /**
         * @brief Evaluate the test.
         *
         * The implementation expands the internal expandable and evaluates the
         * resulting tokens as a test/condition.
         * The command test is used to evaluate the condition, and in case of absence,
         * a new instance of `bs::command_test` will be used.
         *
         * @param oSession Session context used for expansion and evaluation.
         * @return shell_status Resulting status of the test evaluation.
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
          * @brief Get the underlying expandable test node.
          * @return const The underlying expandable test node.
          */
        [[nodiscard]] const shell_node_expandable *get_test() const noexcept {
            return this->m_pTest.get();
        }

    private:
        /// Test expression
        std::unique_ptr<shell_node_expandable> m_pTest;
    };


    /**
     * @class shell_node_if
     * @brief Conditional execution node (if-then-else).
     *
     * Represents an if statement with a condition and two branches: the 'if'
     * branch executed on success and the optional 'else' branch executed on failure.
     */
    class shell_node_if final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct an if node.
         * @throw shell_node_invalid_argument If \p pCondition or \p pCaseIf is null.
         * @param nPos Position in the input stream where the if starts.
         * @param pCondition Owned evaluable node representing the condition.
         * @param pCaseIf Owned evaluable block executed when condition returns success.
         * @param pCaseElse Owned evaluable block executed when condition returns error (may be null).
         */
        shell_node_if(
            std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pCondition,
            std::unique_ptr<shell_node_evaluable> &&pCaseIf,
            std::unique_ptr<shell_node_evaluable> &&pCaseElse
        );

    public:
        /**
         * @brief Evaluate the if statement.
         *
         * The condition is evaluated first. If it returns success, the `pCaseIf`
         * branch is evaluated and its status returned; otherwise `pCaseElse`
         * (if present) is evaluated and its status returned. The exact semantics
         * follow shell evaluation rules for conditionals.
         *
         * @param oSession Session context used for evaluation.
         * @return shell_status Status from the executed branch or the condition.
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
         * @brief Get the condition node.
         * @return const Non-owning pointer to condition.
         */
        [[nodiscard]] const shell_node_evaluable *get_condition() const noexcept {
            return this->m_pCondition.get();
        }

        /**
         * @brief Get the 'if' branch node.
         * @return Non-owning pointer to the 'if' block.
         */
        [[nodiscard]] const shell_node_evaluable *get_case_if() const noexcept {
            return this->m_pCaseIf.get();
        }

        /**
         * @brief Get the 'else' branch node.
         * @return Non-owning pointer to the 'else' block or nullptr.
         */
        [[nodiscard]] const shell_node_evaluable *get_case_else() const noexcept {
            return this->m_pCaseElse.get();
        }

    private:
        /// Owned condition.
        std::unique_ptr<shell_node_evaluable> m_pCondition;
        /// Owned 'if' branch.
        std::unique_ptr<shell_node_evaluable> m_pCaseIf;
        /// Owned 'else' branch (optional).
        std::unique_ptr<shell_node_evaluable> m_pCaseElse;
    };

    /**
     * @class shell_node_continue
     * @brief Represents a `continue` statement inside a loop.
     *
     * When evaluated, this node throws a `continue_signal` exception to unwind
     * the evaluator up to the nearest loop construct (`for`, `while`, `until`).
     * This matches the behavior of shell `continue` statements.
     */
    class shell_node_continue final : public shell_node_evaluable {
    public:
        /**
         * @class continue_signal
         * @brief Exception used internally to signal a loop "continue".
         *
         * The exception is intentionally lightweight and carries no message.
         * It is not intended for user display and is only used for control flow.
         */
        class continue_signal final : public std::runtime_error {
        public:
            /// @brief Construct a continue signal exception.
            continue_signal() : std::runtime_error("") {
            }
        };

    public:
        /**
         * @brief Evaluate the `continue` node.
         *
         * Evaluation immediately throws a `continue_signal` which is expected to be
         * caught by the nearest loop node (`shell_node_for`, `shell_node_while`,
         * `shell_node_until`).
         *
         * @param oSession Session context (unused).
         * @throws shell_node_continue::continue_signal Always thrown.
         * @return Never returns.
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
         * @brief Construct a `continue` node.
         * @param nPos Position in the input stream.
         */
        explicit shell_node_continue(const std::size_t nPos)
            : shell_node(shell_node_type::SNT_CONTINUE, nPos),
              shell_node_evaluable(shell_node_type::SNT_CONTINUE, nPos) {
        }
    };

    /**
     * @class shell_node_break
     * @brief Represents a `break` statement inside a loop.
     *
     * When evaluated, this node throws a `break_signal` exception, allowing
     * the evaluator to unwind to the nearest enclosing loop and terminate it.
     */
    class shell_node_break final : public shell_node_evaluable {
    public:
        /**
         * @class break_signal
         * @brief Exception used internally to signal a loop "break".
         *
         * The exception is intentionally empty and lightweight. It is only for
         * internal control flow and should not be shown to end users.
         */
        class break_signal final : public std::runtime_error {
        public:
            /// @brief Construct a break signal exception.
            break_signal() : std::runtime_error("") {
            }
        };

    public:
        /**
         * @brief Evaluate the `break` node.
         *
         * Evaluation immediately throws a `break_signal`, which is expected to be
         * caught by the nearest loop construct (`for`, `while`, `until`).
         *
         * @param oSession Session context (unused).
         * @throws shell_node_break::break_signal Always thrown.
         * @return Never returns.
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
         * @brief Construct a `break` node.
         * @param nPos Position in the input stream.
         */
        explicit shell_node_break(const std::size_t nPos)
            : shell_node(shell_node_type::SNT_CONTINUE, nPos),
              shell_node_evaluable(shell_node_type::SNT_CONTINUE, nPos) {
        }
    };

    /**
     * @class shell_node_for
     * @brief Iterative 'for' loop node that iterates over an expandable sequence.
     *
     * The loop assigns successive elements of the expanded sequence to shell session variable
     * \c m_sVariable and executes the iterative block for each value.
     */
    class shell_node_for final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct a for-loop node.
         * @throw shell_node_invalid_argument If \p pSequence or \p pIterative is null.
         * @param nPos Position in the input stream where the for loop starts.
         * @param sVariable Name of the loop variable that receives each sequence value (moved in).
         * @param pSequence Owned expandable that yields the sequence to iterate.
         * @param pIterative Owned evaluable block executed for each sequence element.
         */
        shell_node_for(
            std::size_t nPos,
            std::string sVariable,
            std::unique_ptr<shell_node_expandable> &&pSequence,
            std::unique_ptr<shell_node_evaluable> &&pIterative
        );

    public:
        /**
         * @brief Evaluate the for-loop.
         *
         * The sequence is expanded (respecting split semantics). For each token produced,
         * the session variable named by \p m_sVariable is set to that token and \p m_pIterative
         * is executed. The loop follows shell semantics for control (break/continue) and
         * returns the last command status or an appropriate control status.
         *
         * @param oSession Session context used for variable assignment and execution.
         * @return shell_status Status from the loop's execution (last iteration or control status).
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
         * @brief Get the loop variable name.
         * @return Reference to the variable name.
         */
        [[nodiscard]] const std::string &get_variable() const noexcept {
            return this->m_sVariable;
        }

        /**
         * @brief Get the sequence expandable.
         * @return Non-owning pointer to the sequence node.
         */
        [[nodiscard]] const shell_node_expandable *get_sequence() const noexcept {
            return this->m_pSequence.get();
        }

        /**
         * @brief Get the iterative block executed on each element.
         * @return Non-owning pointer to the iterative block.
         */
        [[nodiscard]] const shell_node_evaluable *get_iterative() const noexcept {
            return this->m_pIterative.get();
        }

    private:
        std::string m_sVariable; ///< Loop variable name.
        std::unique_ptr<shell_node_expandable> m_pSequence; ///< Owned sequence provider.
        std::unique_ptr<shell_node_evaluable> m_pIterative; ///< Owned body executed per item.
    };


    /**
     * @class shell_node_while
     * @brief While-loop node executing the iterative block while the condition succeeds.
     *
     * The node repeatedly evaluates the condition and runs the iterative block while the
     * condition returns a success status. Behavior follows shell semantics including support
     * for loop control commands.
     *
     * Ownership: owns both condition and iterative nodes.
     */
    class shell_node_while final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct a while-loop node.
         * @throw shell_node_invalid_argument If \p pCondition or \p pIterative is null.
         * @param nPos Position in the input stream where the while loop starts.
         * @param pCondition Owned evaluable node representing the loop condition.
         * @param pIterative Owned evaluable block executed while the condition succeeds.
         */
        shell_node_while(
            std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pCondition,
            std::unique_ptr<shell_node_evaluable> &&pIterative
        );

    public:
        /**
         * @brief Evaluate the while-loop.
         *
         * Repeatedly evaluates \p m_pCondition and executes \p m_pIterative for as long
         * as the condition returns a success status. Returns the last executed command's
         * status or a control-related status if loop control statements are used.
         *
         * @param oSession Session context used for evaluation.
         * @return shell_status Status from the loop execution.
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
         * @brief Get the condition node.
         * @return Non-owning pointer to the condition.
         */
        [[nodiscard]] const shell_node_evaluable *get_condition() const noexcept {
            return this->m_pCondition.get();
        }

        /**
         * @brief Get the iterative block node.
         * @return Non-owning pointer to the iterative block.
         */
        [[nodiscard]] const shell_node_evaluable *get_iterative() const noexcept {
            return this->m_pIterative.get();
        }

    private:
        /// Owned condition.
        std::unique_ptr<shell_node_evaluable> m_pCondition;
        /// Owned iterative block.
        std::unique_ptr<shell_node_evaluable> m_pIterative;
    };


    /**
     * @class shell_node_until
     * @brief 'Until' loop node that executes the iterative block until the condition succeeds.
     *
     * Semantically inverted from a while loop: the loop continues while the condition fails
     * and stops when the condition returns success. Ownership rules match the while node.
     */
    class shell_node_until final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct an until-loop node.
         * @throw shell_node_invalid_argument If \p pCondition or \p pIterative is null.
         * @param nPos Position in the input stream where the until loop starts.
         * @param pCondition Owned evaluable node representing the stopping condition.
         * @param pIterative Owned evaluable block executed while the condition indicates failure.
         */
        shell_node_until(
            std::size_t nPos,
            std::unique_ptr<shell_node_evaluable> &&pCondition,
            std::unique_ptr<shell_node_evaluable> &&pIterative
        );

    public:
        /**
         * @brief Evaluate the until-loop.
         *
         * Repeatedly evaluates \p m_pCondition and executes \p m_pIterative while the
         * condition returns a non-success status. The loop ends when the condition returns
         * success. The returned status follows shell semantics (last iteration or control).
         *
         * @param oSession Session context used for evaluation.
         * @return shell_status Status from the loop execution or the stopping condition.
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
         * @brief Get the stopping condition node.
         * @return const Non-owning pointer to the condition.
         */
        [[nodiscard]] const shell_node_evaluable *get_condition() const noexcept {
            return this->m_pCondition.get();
        }

        /**
         * @brief Get the iterative block executed while the condition fails.
         * @return Non-owning pointer to the iterative block.
         */
        [[nodiscard]] const shell_node_evaluable *get_iterative() const noexcept {
            return this->m_pIterative.get();
        }

    private:
        /// Owned stopping condition.
        std::unique_ptr<shell_node_evaluable> m_pCondition;
        /// Owned iterative block.
        std::unique_ptr<shell_node_evaluable> m_pIterative;
    };

    /**
     * @class shell_node_function
     * @brief Create function and adds to the shell session.
     */
    class shell_node_function final : public shell_node_evaluable {
    public:
        /**
         * @brief Construct an function node.
         * @throw shell_node_invalid_argument If \p pBody is null.
         * @param nPos Position in the input stream where the until loop starts.
         * @param pName Owned expandable function name.
         * @param pBody Owned evaluable function body.
         */
        shell_node_function(
            std::size_t nPos,
            std::unique_ptr<shell_node_expandable> &&pName,
            std::unique_ptr<shell_node_evaluable> &&pBody
        );

    public:
        /**
         * @brief Adds function to the shell session vtable
         *
         * @param oSession Session context used for evaluation.
         * @return shell_status Status code.
         */
        shell_status evaluate(shell_session &oSession) const override;

    public:
        /**
         * @brief Get the function name node.
         * @return const Non-owning pointer to the name.
         */
        [[nodiscard]] const shell_node_expandable *get_name() const noexcept {
            return this->m_pName.get();
        }

        /**
         * @brief Get the function body node.
         * @return const Non-owning pointer to the body.
         */
        [[nodiscard]] const shell_node_evaluable *get_body() const noexcept {
            return this->m_pBody.get();
        }

    private:
        /// Owned function body.
        std::unique_ptr<shell_node_expandable> m_pName;
        std::unique_ptr<shell_node_evaluable> m_pBody;
    };
} // namespace bs
