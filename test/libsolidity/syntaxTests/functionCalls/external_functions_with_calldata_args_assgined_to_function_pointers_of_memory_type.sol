contract C {
    function f(function (string calldata) external) external {}
    function g(string calldata) external {}

    function main() view external {
        function (string memory) external ptr = this.g;
    }
}
// ----
// Warning 2072: (166-203): Unused local variable.
