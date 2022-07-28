type Int is int;

using {C.add as +} for Int;

contract C {
    function add(int, int) public returns (int) {
        return 0;
    }
}

// ----
// TypeError 4167: (25-30): Only file-level functions and library functions can be bound to a type in a "using" statement
