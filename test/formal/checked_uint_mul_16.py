from opcodes import AND, ISZERO, DIV, MUL, EQ
from rule import Rule
from util import BVUnsignedUpCast
from z3 import BitVec, BitVecVal, Not, BVMulNoOverflow

"""
Overflow checked unsigned integer multiplication.
"""

# Approximation with 16-bit base types.
n_bits = 12

for type_bits in [4, 8, 12]:

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow condition
	actual_overflow = Not(BVMulNoOverflow(X_short, Y_short, False))

	# cast to full n_bits values
	X = BVUnsignedUpCast(X_short, n_bits)
	Y = BVUnsignedUpCast(Y_short, n_bits)
	product = MUL(X, Y)

	# Constants
	bitMask =  BVUnsignedUpCast(BitVecVal((1 << type_bits) - 1, n_bits), n_bits)

	# Overflow check in YulUtilFunction::overflowCheckedIntMulFunctions
	overflow_check = AND(ISZERO(ISZERO(X)), ISZERO(EQ(Y, DIV(AND(product, bitMask), X))))

	rule.check(overflow_check != 0, actual_overflow)
