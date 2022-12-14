# Patch for ESP-IDF

Since some bug fixes of esp-idf may not be synced to GitHub, you need to manually apply some patches to build the example.

## Apply Patch

You can apply the patch by entering the following command on the command line / terminal:

- If you need to use **SDIO NIC** function, you need to apply `idf_patch_x_x.patch`.

    ```
    cd $IDF_PATH
    git apply /path/to/esp-iot-bridge/idf_patch/idf_patch_x_x.patch
    ```

- If you need to communicate between different data forwarding interfaces **(ESP-IDF Release/v5.0)**, you need to apply `ip4_forward.patch`.

    ```
    cd $IDF_PATH/components/lwip/lwip
    git apply /path/to/esp-iot-bridge/idf_patch/ip4_forward.patch
    ```
