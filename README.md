# ESP-KOS-BRIDGE modified from ESP-IOT-Bridge Smart Gateway Solution
<hr>

This version of esp-iot-bridge is focused on wifi extender and timer for gpio pins.
Differences with ESP-IOT-Bridge are our files in main/t3ch_*...

# How to start<br>
1.) Upload to your MCU.<br>
    (Choose correct PORT for your device! (-p) )<br>
    Ex.:<br>
      idf.py menuconfig (OPTIONAL)<br>
      idf.py build      (OPTIONAL)<br>
      idf.py -p /dev/tty.usbserial-54F90067081 -b 115200 flash<br>

2.) connect to wifi ssid: STARTING_SSID, pwd: STARTING_PWD<br>
    ssid & pwd of AP can be changed in t3ch_config.h if necessary before upload or later with HTTP panel.<br>
    
3.) navigate to http://192.168.4.1<br>

4.) configure time, ap, sta, timer... enjoy.<br>

# Supported save of data into NVS. ( If device stays out of power, things still gets saved! )
  - timer settings
  - ap settings
  - sta settings

(Time still needs to be set if device gets out of power! :/)

# Supported HTTPD Requests
  * /free
  * /time/
    - settm
  * /timer/
    - start
    - stop
    - add
    - del
    - stat
  * /wifi/

# Supported uart, jtag, x console commands.. (useful for testing, developing)

# Screenshoots
![alt text](https://github.com/m5it/esp-kos-bridge/blob/main/screenshot_version-0.1.png)

<hr>

# Here is original esp-iot-bridge README.md and location https://github.com/espressif/esp-iot-bridge
# Any questions on ESP-KOS-BRIDGE turn to FB person and developer: Blaz Kos
# Love you all *** :)

