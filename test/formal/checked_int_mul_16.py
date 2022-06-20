from opcodes import AND, SDIV, MUL, EQ, ISZERO, NOT, OR, SGT, SLT, SUB
from rule import Rule
from util import BVSignedUpCast, BVSignedMin
from z3 import BVMulNoOverflow, BVMulNoUnderflow, BitVec, BitVecVal, Not, Or, And

"""
Overflow checked signed integer multiplication.
"""

# Approximation with 16-bit base types.
n_bits = 16

for type_bits in [4, 8, 12]:

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow and underflow conditions
	actual_overflow = Not(BVMulNoOverflow(X_short, Y_short, True))
	actual_underflow = Not(BVMulNoUnderflow(X_short, Y_short))

	# cast to full n_bits values
	X = BVSignedUpCast(X_short, n_bits)
	Y = BVSignedUpCast(Y_short, n_bits)
	product = MUL(X, Y)

	# Constants
	bit_mask = BitVecVal(2**type_bits - 1, n_bits)
	sign_mask = BitVecVal(2**(type_bits-1), n_bits)
	min_value = BVSignedMin(type_bits, n_bits)

	# Overflow and underflow checks in YulUtilFunction::overflowCheckedIntMulFunction
	if type_bits == n_bits:
		sol_overflow_check_1 = AND(EQ(X, SUB(0, 1)), EQ(Y, min_value))
		sol_overflow_check_2 = AND(ISZERO(ISZERO(X)), ISZERO(EQ(Y, SDIV(product, X))))
	else:
		sol_overflow_check_1 = AND(
			ISZERO(AND(product, sign_mask)),
			AND(ISZERO(ISZERO(X)), ISZERO(EQ(Y, SDIV(AND(product, bit_mask), X))))
		)
		sol_overflow_check_2 = AND(
			ISZERO(ISZERO(AND(product, sign_mask))),
			AND(ISZERO(ISZERO(X)), ISZERO(EQ(Y, SDIV(OR(product, NOT(bit_mask)), X))))
		)

	# product sign check needed because z3 distinguishes underflow from overflow
	positive_product = OR(AND(SGT(X, 0), SGT(Y, 0)), AND(SLT(X, 0), SLT(Y, 0)))
	negative_product = OR(AND(SGT(X, 0), SLT(Y, 0)), AND(SLT(X, 0), SGT(Y, 0)))

	# specific tests for underflow and overflow
	formal_overflow_check =  And(positive_product != 0,	Or(sol_overflow_check_1 != 0, sol_overflow_check_2 != 0))
	formal_underflow_check = And(negative_product != 0, Or(sol_overflow_check_1 != 0, sol_overflow_check_2 != 0))

	rule.check(actual_overflow, formal_overflow_check)
	rule.check(actual_underflow, formal_underflow_check)
