// SPDX-License-Identifier: GPL-3.0-only
pragma solidity 0.8.20;

contract HorseStoreYul {
    uint256 numberOfHorses;

    constructor() payable {}

    function updateHorseNumber(uint256 newNumberOfHorses) external {
        assembly {
            sstore(numberOfHorses.slot, newNumberOfHorses)
        }
    }

    function readNumberOfHorses() external view returns (uint256) {
        assembly {
            let num := sload(numberOfHorses.slot)

            mstore(0x80, num)
            return(0x80, 0x20)
        }
    }
}
