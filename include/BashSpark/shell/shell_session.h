/**
 * @file shell_session.h
 * @brief Defines the class `bs::shell_session`.
 *
 * This file contains the definition of the `bs::shell_session` class,
 * which provides the "running environment" for the `bs::shell`.
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

#include <functional>
#include <iosfwd>
#include <sstream>
#include <memory>

#include "shell_vtable.h"
#include "BashSpark/shell/shell_arg.h"
#include "BashSpark/shell/shell_env.h"
#include "BashSpark/shell/shell_var.h"
#include "BashSpark/shell/shell_status.h"

namespace bs {
    class shell;

    /**
     * @class shell_session
     * @brief Represents an execution environment for a shell instance.
     *
     * A `shell_session` encapsulates the streams (stdin/stdout/stderr), the
     * environment variables (`shell_env`), local shell variables (`shell_var`),
     * and argument lists (`shell_arg`) used by the running shell.
     *
     * It provides the interface for creating subsessions, function-call sessions,
     * and pipe-left/right sessions.
     */
    class shell_session {
    public:
        /// Sell function type
        using func_type = shell_vtable::func_type;

    public:
        /**
         * @brief Construct a new shell session with fresh env/var/arg.
         *
         * @param pShell Pointer to the associated shell.
         * @param oStdIn Input stream for the session.
         * @param oStdOut Output stream for the session.
         * @param oStdErr Error stream for the session.
         */
        shell_session(
            const shell *pShell,
            std::istream &oStdIn,
            std::ostream &oStdOut,
            std::ostream &oStdErr
        )
            : m_pEnv(std::make_shared<shell_env>()),
              m_pArg(std::make_shared<shell_arg>()),
              m_pVar(std::make_shared<shell_var>()),
              m_pVtable(std::make_shared<shell_vtable>()),
              m_nLastCommandResult(shell_status::SHELL_SUCCESS),
              m_pShell(pShell),
              m_oStdIn(oStdIn),
              m_oStdOut(std::ref(oStdOut)),
              m_oStdErr(std::ref(oStdErr)) {
        }

        /**
         * @brief Construct a new shell session providing initial env and args.
         *
         * @param pBash Pointer to the associated shell.
         * @param oStdIn Input stream for the session.
         * @param oStdOut Output stream for the session.
         * @param oStdErr Error stream for the session.
         * @param oEnv Initial environment object.
         * @param oArg Initial argument object.
         */
        shell_session(
            const shell *pBash,
            std::istream &oStdIn,
            std::ostream &oStdOut,
            std::ostream &oStdErr,
            shell_env oEnv,
            shell_arg oArg
        )
            : m_pEnv(std::make_shared<shell_env>(std::move(oEnv))),
              m_pArg(std::make_shared<shell_arg>(std::move(oArg))),
              m_pVar(std::make_shared<shell_var>()),
              m_nLastCommandResult(shell_status::SHELL_SUCCESS),
              m_pShell(pBash),
              m_oStdIn(oStdIn),
              m_oStdOut(std::ref(oStdOut)),
              m_oStdErr(std::ref(oStdErr)) {
        }

        /// @brief Virtual destructor for subclassing.
        virtual ~shell_session() = default;

        // @section shell_pointer Shell Pointer

        /**
         * @brief Get the pointer to the underlying shell.
         * @return const shell* The associated shell instance.
         */
        [[nodiscard]] const shell *get_shell() const noexcept {
            return m_pShell;
        }

        // @section shell_env Environment

        /** @brief Get modifiable environment. */
        [[nodiscard]] shell_env &env() noexcept { return *m_pEnv; }

        /** @brief Get const environment. */
        [[nodiscard]] const shell_env &env() const noexcept { return *m_pEnv; }

        /**
         * @brief Check whether an environment variable exists.
         * @param sVariable Variable name.
         * @return true if the variable exists.
         */
        [[nodiscard]] bool has_env(const std::string &sVariable) const noexcept {
            return m_pEnv->has_env(sVariable);
        }

        /**
         * @brief Retrieve an environment variable.
         * @param sVariable Variable name.
         * @return The value of the variable, or empty string if nonexistent.
         */
        [[nodiscard]] std::string get_env(const std::string &sVariable) const {
            return m_pEnv->get_env(sVariable);
        }

        /**
         * @brief Retrieve an environment variable using hop-2 resolution.
         *
         * Hop-2 resolution is implementation-dependent behavior from shell_env.
         *
         * @param sVariable Variable name.
         * @return The resolved value.
         */
        [[nodiscard]] std::string get_env_hop2(const std::string &sVariable) const {
            return m_pEnv->get_env_hop2(sVariable);
        }

        /**
         * @brief Set an environment variable.
         * @param sVariable Variable name.
         * @param sValue Value to assign.
         */
        void set_env(const std::string &sVariable, std::string sValue) {
            m_pEnv->set_env(sVariable, std::move(sValue));
        }

        // @section shell_var Variables

        /** @brief Get const variable table. */
        [[nodiscard]] const shell_var &var() const noexcept { return *m_pVar; }

        /** @brief Get modifiable variable table. */
        [[nodiscard]] shell_var &var() noexcept { return *m_pVar; }

        /**
         * @brief Check whether a local variable exists.
         * @param sVariable Variable name.
         */
        [[nodiscard]] bool has_var(const std::string &sVariable) const noexcept {
            return m_pVar->has_var(sVariable);
        }

        /**
         * @brief Retrieve a local variable.
         * @param sVariable Variable name.
         */
        [[nodiscard]] std::string get_var(const std::string &sVariable) const {
            return m_pVar->get_var(sVariable);
        }

        /**
         * @brief Retrieve a local variable using hop-2 resolution.
         * @param sVariable Variable name.
         */
        [[nodiscard]] std::string get_var_hop2(const std::string &sVariable) const {
            return m_pVar->get_var_hop2(sVariable);
        }

        /**
         * @brief Set a local variable.
         * @param sVariable Variable name.
         * @param sValue Assigned value.
         */
        void set_var(const std::string &sVariable, std::string sValue) {
            m_pVar->set_var(sVariable, std::move(sValue));
        }

        // @section shell_arg Arguments

        /** @brief Get const argument list. */
        [[nodiscard]] const shell_arg &arg() const noexcept { return *m_pArg; }

        /** @brief Get modifiable argument list. */
        [[nodiscard]] shell_arg &arg() noexcept { return *m_pArg; }

        /**
         * @brief Retrieve a specific argument.
         * @param nArg Index of argument.
         * @return Argument string (empty if out of bounds).
         */
        [[nodiscard]] std::string get_arg(const std::size_t nArg) const {
            return m_pArg->get_arg(nArg);
        }

        /**
         * @brief Get number of arguments.
         */
        [[nodiscard]] std::size_t get_arg_size() const {
            return m_pArg->get_arg_size();
        }

        /**
         * @brief Get the full argument list.
         * @return const reference to vector of strings.
         */
        [[nodiscard]] const std::vector<std::string> &get_args() const noexcept {
            return m_pArg->get_args();
        }

        // @section streams Streams

        /** @brief Get stdin stream. */
        [[nodiscard]] std::istream &in() const noexcept { return m_oStdIn; }

        /** @brief Get stdout stream. */
        [[nodiscard]] std::ostream &out() const noexcept { return m_oStdOut; }

        /** @brief Get stderr stream. */
        [[nodiscard]] std::ostream &err() const noexcept { return m_oStdErr; }

        // @section shell_status Status

        /**
         * @brief Get status of last executed command.
         */
        [[nodiscard]] shell_status get_last_command_result() const noexcept {
            return m_nLastCommandResult;
        }

        /**
         * @brief Set status of last executed command.
         */
        void set_last_command_result(const shell_status nLastCommandResult) noexcept {
            m_nLastCommandResult = nLastCommandResult;
        }

        // @section shell_subsession Subsession creation

        /**
         * @brief Create a subsession with fresh streams but copied env/args/vars.
         *
         * Used for subshells. Implementations may extend the stored data,
         * such as by adding a working directory.
         *
         * @param oStdIn New input stream.
         * @param oStdOut New output stream.
         * @param oStdErr New error stream.
         *
         * @return Newly allocated independent session.
         */
        virtual std::unique_ptr<shell_session> make_subsession(
            std::istream &oStdIn,
            std::ostream &oStdOut,
            std::ostream &oStdErr
        ) {
            return std::unique_ptr<shell_session>(new shell_session(
                m_pShell,
                oStdIn,
                oStdOut,
                oStdErr,
                std::make_shared<shell_env>(*m_pEnv),
                m_pArg,
                std::make_shared<shell_var>(*m_pVar),
                std::make_shared<shell_vtable>(*m_pVtable)
            ));
        }

        /**
         * @brief Create a session for function calls.
         *
         * Keeps env and streams but provides new arguments and a new variable table.
         *
         * @param oArg Argument list to use for the function call.
         */
        virtual std::unique_ptr<shell_session> make_function_call(shell_arg oArg) {
            std::cout << "fcall " << m_pArg->get_arg(1) << std::endl;
            return std::unique_ptr<shell_session>(new shell_session(
                m_pShell, m_oStdIn, m_oStdOut, m_oStdErr,
                m_pEnv,
                std::make_shared<shell_arg>(std::move(oArg)),
                std::make_shared<shell_var>(),
                m_pVtable
            ));
        }

        /**
         * @brief Create a session representing the left side of a pipe.
         *
         * Output is redirected to the given ostringstream.
         *
         * @param oStdOut Output buffer receiving piped data.
         */
        virtual std::unique_ptr<shell_session> make_pipe_left(
            std::ostringstream &oStdOut
        ) {
            return std::unique_ptr<shell_session>(new shell_session(
                m_pShell, m_oStdIn, oStdOut, m_oStdErr,
                m_pEnv, m_pArg, m_pVar, m_pVtable
            ));
        }

        /**
         * @brief Create a session representing the right side of a pipe.
         *
         * Input is taken from the given istringstream.
         *
         * @param oStdIn Input buffer from previous pipe stage.
         */
        virtual std::unique_ptr<shell_session> make_pipe_right(
            std::istringstream &oStdIn
        ) {
            return std::unique_ptr<shell_session>(new shell_session(
                m_pShell,
                oStdIn,
                m_oStdOut,
                m_oStdErr,
                m_pEnv,
                m_pArg,
                m_pVar,
                m_pVtable
            ));
        }

        // @section vtable Function vtable

        /**
         * @brief Gets a function
         * @param sVar The name of the function to get
         * @return The function if there is or nullptr
         */
        [[nodiscard]] const func_type *get_func(const std::string &sVar) const {
            return this->m_pVtable->get_func(sVar);
        }

        /**
         * @brief Sets the a function.
         * @param sName The name of the function.
         * @param pFunction The function to be set.
         */
        void set_func(const std::string &sName, const func_type *pFunction) {
            this->m_pVtable->set_func(sName, pFunction);
        }

        /**
         * @brief Checks if a function exists.
         * @param sName The name of the function.
         * @return True if the function exists, false otherwise.
         */
        [[nodiscard]] bool has_func(const std::string &sName) const noexcept {
            return this->m_pVtable->has_func(sName);
        }

        /**
         * @brief Gets the number of functions in the vtable.
         * @return The size of the function map.
         */
        [[nodiscard]] std::size_t get_vtable_size() const noexcept {
            return this->m_pVtable->get_vtable_size();
        }

       // @section depth Shell Depth

    public:
        /**
         * @brief Gets the current shell depth
         * @return Current shell depth
         */
        [[nodiscard]] std::size_t get_current_shell_depth() const noexcept {
            return this->m_nCurrentDepth;
        }

        /**
         * @brief Decreases the current shell depth
         */
        void decrease_shell_depth() noexcept {
            if (this->m_nCurrentDepth > 0)this->m_nCurrentDepth--;
        }

        /**
         * @brief Tries to increase the shell depth
         * @return Whether the shell depth could be increased
         */
        bool increase_shell_depth() noexcept {
            if (this->m_nCurrentDepth + 1 <= SHELL_MAX_DEPTH) {
                this->m_nCurrentDepth++;
                return true;
            }
            return false;
        }

    protected:
        /**
         * @brief Protected constructor used by session-creation helpers.
         */
        shell_session(
            const shell *pBash,
            std::istream &oStdIn,
            std::ostream &oStdOut,
            std::ostream &oStdErr,
            std::shared_ptr<shell_env> pEnv,
            std::shared_ptr<shell_arg> pArg,
            std::shared_ptr<shell_var> pVar,
            std::shared_ptr<shell_vtable> pVtable
        )
            : m_pEnv(std::move(pEnv)),
              m_pArg(std::move(pArg)),
              m_pVar(std::move(pVar)),
              m_pVtable(std::move(pVtable)),
              m_nLastCommandResult(shell_status::SHELL_SUCCESS),
              m_pShell(pBash),
              m_oStdIn(std::ref(oStdIn)),
              m_oStdOut(std::ref(oStdOut)),
              m_oStdErr(std::ref(oStdErr)) {
        }

    protected:
        /// Shared environment.
        std::shared_ptr<shell_env> m_pEnv;
        /// Argument list.
        std::shared_ptr<shell_arg> m_pArg;
        /// Local variables.
        std::shared_ptr<shell_var> m_pVar;
        /// Function vTable.
        std::shared_ptr<shell_vtable> m_pVtable;
        /// Last return status.
        shell_status m_nLastCommandResult;

    private:
        /// Pointer to owning shell.
        const shell *m_pShell;
        /// Session stdin.
        std::reference_wrapper<std::istream> m_oStdIn;
        /// Session stdout.
        std::reference_wrapper<std::ostream> m_oStdOut;
        /// Session stderr.
        std::reference_wrapper<std::ostream> m_oStdErr;
        /// Shell depth
        std::size_t m_nCurrentDepth = 0;
    };
} // namespace bs
