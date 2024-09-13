// SPDX-License-Identifier: SEE LICENSE IN LICENSE
pragma solidity 0.8.20;
import {HorseStroeBaseTest, HorseStore} from "./HorseStoreBase.sol";
import {HuffDeployer} from "foundry-huff/HuffDeployer.sol";

contract horseStoreHuffTest is HorseStroeBaseTest {
    string public constant HORSE_STORE_HUFF_LOCATION =
        "HorseStoreV1/horseStore";

    function setUp() external override {
        horseStore = HorseStore(
            HuffDeployer.config().deploy(HORSE_STORE_HUFF_LOCATION)
        );
    }
}
