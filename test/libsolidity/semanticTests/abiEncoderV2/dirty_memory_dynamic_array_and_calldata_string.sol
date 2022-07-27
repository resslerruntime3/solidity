pragma abicoder v2;

contract C
{
    struct Data
    {
        uint256[] a;
        string b;
    }

    function makeDirty() internal
    {
        uint memStart = 0;
        assembly { memStart := mload(0x40) }
        uint memEnd = memStart + 0x800;

        for (uint pos = memStart; pos < memEnd; pos += 0x20)
            assembly { mstore(pos, not(0)) }
    }

    function makeTestData() internal returns (Data memory data)
    {
        data.a = new uint256[](1);
        data.a[0] = 0xFF;
        data.b = "123456";
    }

    function makeTestArray() internal returns (uint256[] memory array)
    {
        array = new uint256[](2);
        array[0] = 0xAF;
        array[1] = 0xBF;
    }

    function testDirtyCalldata() public returns (bytes memory)
    {
        makeDirty();

        return this.calldataTest(makeTestData(), makeTestArray());
    }

    function testDirtyMemory() public returns (bytes memory)
    {
        makeDirty();

        return this.memoryTest(makeTestData(), makeTestArray());
    }

    function calldataTest(Data calldata data, uint256[] calldata a) public pure returns (bytes memory)
    {
        return abi.encode(data, a);
    }

    function memoryTest(Data memory data, uint256[] calldata a) public pure returns (bytes memory)
    {
        return abi.encode(data, a);
    }
}

// ====
// EVMVersion: >homestead
// ----
// testDirtyCalldata() -> 0x20, 0x0160, 0x40, 0x0100, 0x40, 0x80, 1, 0xff, 6, "123456", 2, 0xaf, 0xbf
// gas irOptimized: 186388
// gas legacy: 546971
// gas legacyOptimized: 306672
// calldataTest((uint256[],string),uint256[]): 0x40, 0x100, 0x40, 0x80, 1, 0xFF, 6, "123456XXX", 2, 0xAF, 0xBF -> 0x20, 0x0160, 0x40, 0x0100, 0x40, 0x80, 1, 0xff, 6, "123456", 2, 0xaf, 0xbf
// testDirtyMemory() -> 0x20, 0x0160, 0x40, 0x0100, 0x40, 0x80, 1, 0xff, 6, "123456", 2, 0xaf, 0xbf
// gas irOptimized: 187335
// gas legacy: 548903
// gas legacyOptimized: 307555
// memoryTest((uint256[],string),uint256[]): 0x40, 0x100, 0x40, 0x80, 1, 0xFF, 6, "123456XXX", 2, 0xAF, 0xBF -> 0x20, 0x0160, 0x40, 0x0100, 0x40, 0x80, 1, 0xff, 6, "123456", 2, 0xaf, 0xbf
