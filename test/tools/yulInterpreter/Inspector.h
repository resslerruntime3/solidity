/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0
/**
 * Yul inspector.
 */

#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/AST.h>

#include <string>

#pragma once

namespace solidity::yul::test
{

class Inspector
{
public:
	Inspector(std::string const& _source, InterpreterState const& _state)
		:m_source(_source), m_state(_state) {}

	bool queryUser(DebugData const& _data, std::map<YulString, u256> const& _variables);

	void resetSkipSteps() { m_skipSteps = false; }
	std::string const& source() const { return m_source; }
private:
	// Source of the file
	std::string const& m_source;
	InterpreterState const& m_state;

	std::string m_lastInput;
	bool m_skipSteps = false;
	std::set<langutil::SourceLocation> m_breakpoints;
};


class InspectedInterpreter: public Interpreter
{
public:
	static void run(
		std::shared_ptr<Inspector> _inspector,
		InterpreterState& _state,
		Dialect const& _dialect,
		Block const& _ast,
		bool _disableMemoryTracing
	);

	InspectedInterpreter(
		std::shared_ptr<Inspector> _inspector,
		InterpreterState& _state,
		Dialect const& _dialect,
		Scope& _scope,
		bool _disableMemoryTracing,
		std::map<YulString, u256> _variables = {}
	):
		Interpreter(_state, _dialect, _scope, _disableMemoryTracing, _variables),
		m_inspector(_inspector)
	{
	}

	template <typename ConcreteNode>
	void helper(ConcreteNode const& _node)
	{
		bool skipped = m_inspector->queryUser(*_node.debugData, m_variables);
		Interpreter::operator()(_node);
		if (skipped)
			m_inspector->resetSkipSteps();
	}

	void operator()(ExpressionStatement const& _node) override { helper(_node); }
	void operator()(Assignment const& _node) override { helper(_node); }
	void operator()(VariableDeclaration const& _node) override { helper(_node); }
	void operator()(If const& _node) override { helper(_node); }
	void operator()(Switch const& _node) override { helper(_node); }
	void operator()(ForLoop const& _node) override { helper(_node); }
	void operator()(Break const& _node) override { helper(_node); }
	void operator()(Continue const& _node) override { helper(_node); }
	void operator()(Leave const& _node) override { helper(_node); }
	void operator()(Block const& _node) override { helper(_node); }
protected:
	/// Asserts that the expression evaluates to exactly one value and returns it.
	u256 evaluate(Expression const& _expression) override;
	/// Evaluates the expression and returns its value.
	std::vector<u256> evaluateMulti(Expression const& _expression) override;
private:
	std::shared_ptr<Inspector> m_inspector;
};


class InspectedExpressionEvaluator: public ExpressionEvaluator
{
public:
	InspectedExpressionEvaluator(
		std::shared_ptr<Inspector> _inspector,
		InterpreterState& _state,
		Dialect const& _dialect,
		Scope& _scope,
		std::map<YulString, u256> const& _variables,
		bool _disableMemoryTrace
	):
		ExpressionEvaluator(_state, _dialect, _scope, _variables, _disableMemoryTrace),
		m_inspector(_inspector)
	{}

	template <typename ConcreteNode>
	void helper(ConcreteNode const& _node)
	{
		bool skipped = m_inspector->queryUser(*_node.debugData, m_variables);
		ExpressionEvaluator::operator()(_node);
		if (skipped)
			m_inspector->resetSkipSteps();
	}

	void operator()(Literal const& _node) override { helper(_node); }
	void operator()(Identifier const& _node) override { helper(_node); }
	void operator()(FunctionCall const& _node) override { helper(_node); }
protected:
	std::shared_ptr<Interpreter> makeInterpreterCopy(std::map<YulString, u256> _variables = {}) const override
	{
		return std::make_shared<InspectedInterpreter>(
			m_inspector,
			m_state,
			m_dialect,
			m_scope,
			m_disableMemoryTrace,
			std::move(_variables)
		);
	}
	std::shared_ptr<Interpreter> makeInterpreterNew(InterpreterState& _state, Scope& _scope) const override
	{
		return std::make_shared<InspectedInterpreter>(
			std::make_shared<Inspector>(
				m_inspector->source(),
				_state
			),
			_state,
			m_dialect,
			_scope,
			m_disableMemoryTrace
		);
	}
private:
	std::shared_ptr<Inspector> m_inspector;
};

}
