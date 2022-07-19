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
 * Yul interpreter.
 */

#include <test/tools/yulInterpreter/Inspector.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test;

using namespace std;

namespace
{

void printVariable(YulString const& _name, u256 const& _value)
{
	cout << "\t" << _name.str() << " = " << _value.str();

	if (_value != 0)
		cout << " (" << toCompactHexWithPrefix(_value) << ")";

	cout << endl;
}

}

void InspectedInterpreter::run(
	std::shared_ptr<Inspector> _inspector,
	InterpreterState& _state,
	Dialect const& _dialect,
	Block const& _ast,
	bool _disableMemoryTrace
)
{
	Scope scope;
	InspectedInterpreter{_inspector, _state, _dialect, scope, _disableMemoryTrace}(_ast);
}

bool Inspector::queryUser(DebugData const& _data, map<YulString, u256> const& _variables)
{
	if (
		!m_breakpoints.count(_data.nativeLocation) &&
		m_skipSteps
	)
	{
		// Output instructions that are being skipped/run
		cout << "Running "
			<< m_source.substr((uint)_data.nativeLocation.start, (uint)_data.nativeLocation.end - (uint)_data.nativeLocation.start)
			<< endl;
		return false;
	}

	string input;

	while (true)
	{
		// Output sourcecode about to run.
		cout << "> "
			<< m_source.substr((uint)_data.nativeLocation.start, (uint)_data.nativeLocation.end - (uint)_data.nativeLocation.start)
			<< endl;

		// Ask user for action
		cout << endl
			<< "(s)tep/(n)ext/(i)nspect/(p)rint/all (v)ariables?"
			<< endl
			<< "# ";

		cout.flush();

		getline(cin, input);
		boost::algorithm::trim(input);

		if (input == "")
			input = m_lastInput;

		if (input == "next" || input == "n")
		{
			m_lastInput = input;
			m_skipSteps = true;
			return true;
		}
		else if (input == "step" || input == "s")
		{
			m_lastInput = input;
			return false;
		}
		else if (input == "inspect" || input == "i")
		{
			m_lastInput = input;
			m_state.dumpTraceAndState(cout, false);
		}
		else if (input == "variables" || input == "v")
		{
			m_lastInput = input;
			for (auto &&[yulStr, val]: _variables)
				printVariable(yulStr, val);
			cout << endl;
		}
		else if (
			boost::starts_with(input, "print") ||
			boost::starts_with(input, "p")
		)
		{
			m_lastInput = input;

			size_t whitespacePos = input.find(' ');

			if (whitespacePos == string::npos)
				cout << "Error parsing command! Expected variable name." << endl;

			string const varname = input.substr(whitespacePos + 1);

			vector<string> candidates;

			bool found = false;
			for (auto &&[yulStr, val]: _variables)
				if (yulStr.str() == varname)
				{
					printVariable(yulStr, val);
					found = true;
					break;
				}

			if (!found)
				cout << varname << " not found." << endl;

		}
	}
}

u256 InspectedInterpreter::evaluate(Expression const& _expression)
{
	InspectedExpressionEvaluator ev(m_inspector, m_state, m_dialect, *m_scope, m_variables, m_disableMemoryTrace);
	ev.visit(_expression);
	return ev.value();
}

std::vector<u256> InspectedInterpreter::evaluateMulti(Expression const& _expression)
{
	InspectedExpressionEvaluator ev(m_inspector, m_state, m_dialect, *m_scope, m_variables, m_disableMemoryTrace);
	ev.visit(_expression);
	return ev.values();
}
