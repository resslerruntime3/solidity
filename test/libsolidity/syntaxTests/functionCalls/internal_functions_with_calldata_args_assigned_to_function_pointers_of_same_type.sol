contract C {
    function g(bytes calldata b) pure internal returns (bytes calldata) {
        return b[2:5];
    }

    function main() pure external {
        function (bytes calldata) internal returns (bytes calldata) ptr = g;
    }
}
// ----
// Warning 2072: (161-224): Unused local variable.
