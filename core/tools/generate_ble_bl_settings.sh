nrfutil settings generate --family NRF52 --bootloader-version 0 --bl-settings-version 2 --app-boot-validation VALIDATE_ECDSA_P256_SHA256 --application ./build/ble_firmware/ble_firmware.hex --application-version 0 --sd-boot-validation VALIDATE_ECDSA_P256_SHA256 --softdevice ./embed/sdk/nrf52/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex --key-file ./embed/ble_bootloader/priv.pem ./build/ble_bootloader/settings.hex