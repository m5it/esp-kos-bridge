# ESP-KOS-BRIDGE modified from ESP-IOT-Bridge Smart Gateway Solution
<hr>

This version of esp-iot-bridge is focused on wifi extender.

# How to start
1.) connect to wifi ssid: STARTING_SSID, pwd: STARTING_PWD<br>
2.) navigate to http://192.168.4.1<br>
3.) configure time, ap, sta, timer... enjoy.<br>

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

# Screenshoots
![alt text](https://github.com/m5it/esp-kos-bridge/blob/main/screenshot_version-0.1.png)

<hr>
# Here is original esp-iot-bridge README.md and location https://github.com/espressif/esp-iot-bridge
# Any questions on ESP-KOS-BRIDGE turn to FB person and developer: Blaz Kos
# Love you all *** :)

