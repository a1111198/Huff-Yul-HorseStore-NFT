// SPDX-License-Identifier: MIT
pragma solidity 0.8.20;

import {Test} from "forge-std/Test.sol";
import {HorseStore} from "../../src/HorseStoreV1/horseStore.sol";
import {console} from "forge-std/console.sol";

abstract contract HorseStroeBaseTest is Test {
    HorseStore horseStore;

    function setUp() external virtual {
        horseStore = new HorseStore();
        console.log(gasleft());
    }

    function testReadHorseStore() external view {
        uint256 initialHorses = horseStore.readNumberOfHorses();
        assertEq(initialHorses, 0);
    }

    function testWriteHorseStore(uint256 _numOfHorses) external {
        horseStore.updateHorseNumber(_numOfHorses);
        assertEq(horseStore.readNumberOfHorses(), _numOfHorses);
    }
}
