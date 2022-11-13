#ifndef FALLOUT_GAME_ELEVATOR_H_
#define FALLOUT_GAME_ELEVATOR_H_

typedef enum Elevator {
    ELEVATOR_BROTHERHOOD_OF_STEEL_MAIN,
    ELEVATOR_BROTHERHOOD_OF_STEEL_SURFACE,
    ELEVATOR_MASTER_UPPER,
    ELEVATOR_MASTER_LOWER,
    ELEVATOR_MILITARY_BASE_UPPER,
    ELEVATOR_MILITARY_BASE_LOWER,
    ELEVATOR_GLOW_UPPER,
    ELEVATOR_GLOW_LOWER,
    ELEVATOR_VAULT_13,
    ELEVATOR_NECROPOLIS,
    ELEVATOR_SIERRA_1,
    ELEVATOR_SIERRA_2,
    ELEVATOR_SIERRA_SERVICE,
    ELEVATOR_KLAMATH_TOXIC_CAVES,
    ELEVATOR_14,
    ELEVATOR_VAULT_CITY,
    ELEVATOR_VAULT_15_MAIN,
    ELEVATOR_VAULT_15_SURFACE,
    ELEVATOR_NAVARRO_NORTHERN,
    ELEVATOR_NAVARRO_CENTER,
    ELEVATOR_NAVARRO_LAB,
    ELEVATOR_NAVARRO_CANTEEN,
    ELEVATOR_SAN_FRANCISCO_SHI_TEMPLE,
    ELEVATOR_REDDING_WANAMINGO_MINE,
    ELEVATOR_COUNT,
} Elevator;

int elevator_select(int elevator, int* mapPtr, int* elevationPtr, int* tilePtr);

#endif /* FALLOUT_GAME_ELEVATOR_H_ */
