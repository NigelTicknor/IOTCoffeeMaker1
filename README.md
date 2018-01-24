# IOTCoffeeMaker1
Short ESP8266 arduino code to make a coffee maker turn on/off over WiFi.
The model I used is a Mr. Coffee DRX5.

The coffee maker will also POST to a URL whenever an action is performed.

This is meant for a coffee maker that has on/off buttons that run on low (<=3.3) voltages. These can be found on programmable coffee makers and possibly others. If your coffee maker has a single button for virtual toggle (doesn't physically stay depressed), you can set PIN_BUTTON_ON and PIN_BUTTON_OFF to the same pin.

