#include "flash.h"
#include "model.h"

const flash_area_t STORAGE_AREAS[STORAGE_AREAS_COUNT] = {
    {
        .num_subareas = 1,
        .subarea[0] =
            {
                .first_sector = STORAGE_1_SECTOR_START,
                .num_sectors =
                    STORAGE_1_SECTOR_END - STORAGE_1_SECTOR_START + 1,
            },
    },
    {
        .num_subareas = 1,
        .subarea[0] =
            {
                .first_sector = STORAGE_2_SECTOR_START,
                .num_sectors =
                    STORAGE_2_SECTOR_END - STORAGE_2_SECTOR_START + 1,
            },
    },
};

const flash_area_t BOARDLOADER_AREA = {
    .num_subareas = 1,
    .subarea[0] =
        {
            .first_sector = BOARDLOADER_SECTOR_START,
            .num_sectors =
                BOARDLOADER_SECTOR_END - BOARDLOADER_SECTOR_START + 1,
        },
};

const flash_area_t SECRET_AREA = {
    .num_subareas = 1,
    .subarea[0] =
        {
            .first_sector = 12,
            .num_sectors = 1,
        },
};

const flash_area_t TRANSLATIONS_AREA = {
    .num_subareas = 1,
    .subarea[0] =
        {
            .first_sector = 13,
            .num_sectors = 2,
        },
};

const flash_area_t BOOTLOADER_AREA = {
    .num_subareas = 1,
    .subarea[0] =
        {
            .first_sector = BOOTLOADER_SECTOR_START,
            .num_sectors = BOOTLOADER_SECTOR_END - BOOTLOADER_SECTOR_START + 1,
        },
};

const flash_area_t FIRMWARE_AREA = {
    .num_subareas = 2,
    .subarea[0] =
        {
            .first_sector = FIRMWARE_SECTOR_START,
            .num_sectors = FIRMWARE_SECTOR_END - FIRMWARE_SECTOR_START + 1,
        },
    .subarea[1] =
        {
            .first_sector = FIRMWARE_P2_SECTOR_START,
            .num_sectors =
                FIRMWARE_P2_SECTOR_END - FIRMWARE_P2_SECTOR_START + 1,
        },
};

const flash_area_t UNUSED_AREA = {
    .num_subareas = 2,
    .subarea[0] =
        {
            .first_sector = 3,
            .num_sectors = 1,
        },
    .subarea[1] =
        {
            .first_sector = 15,
            .num_sectors = 1,
        },
};
