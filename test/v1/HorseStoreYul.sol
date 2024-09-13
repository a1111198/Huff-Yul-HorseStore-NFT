// SPDX-License-Identifier: SEE LICENSE IN LICENSE
pragma solidity 0.8.20;
import {HorseStroeBaseTest, HorseStore} from "./HorseStoreBase.sol";
import {HorseStoreYul} from "../../src/HorseStoreV1/yulHorseStore.sol";

contract horseStoreYulTest is HorseStroeBaseTest {
    function setUp() external override {
        horseStore = HorseStore(address(new HorseStoreYul()));
    }
}
