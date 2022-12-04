# Arduino CheerLights

### Sketches
|**Name**|**Protocol**|**Sketch Link**|**Notes**|**Hardware**|
|---|---|---|---|---|
|ESP8266_SingleRGBLED|HTTP|[<b>Sketch</b>](https://github.com/jfrmilner/Arduino-CheerLights/blob/master/Sketches/CheerLights_ESP8266_SingleRGBLED/CheerLights_ESP8266_SingleRGBLED.ino)|| [ESP8266](https://wiki.wemos.cc/products:d1:d1_mini) + [Common Anode RGB LED](https://www.sparkfun.com/products/10820)|
|ESP8266_SingleRGBLED|MQTT|[<b>Sketch</b>](https://github.com/jfrmilner/Arduino-CheerLights/blob/master/Sketches/CheerLights_ESP8266_SingleRGBLED_MQTT/CheerLights_ESP8266_SingleRGBLED_MQTT.ino)|| [ESP8266](https://wiki.wemos.cc/products:d1:d1_mini) + [Common Anode RGB LED](https://www.sparkfun.com/products/10820)|


# Demos
## ESP8266 with single common anode/cathode RGB LED version (Easy)
This demo uses the [ESP8266_SingleRGBLED](https://github.com/jfrmilner/Arduino-CheerLights/blob/master/Sketches/CheerLights_ESP8266_SingleRGBLED_MQTT/CheerLights_ESP8266_SingleRGBLED_MQTT.ino) sketch with the MQTT protocol. Here we can see a tweet for “Red Green Purple #cheerlights” and how it changes two separate trees each with their own microcontroller, notice how they’re in perfect sync and no colours were missed – this is a key advantage over HTTP polling! 



https://user-images.githubusercontent.com/3640168/205512752-217b85a8-deb1-4284-80e5-d19a2acb4873.mp4



## ESP8266 with multiple RGB LEDs (Addressable Strip/Pixels) (FastLED) version (Advanced)
Colour Wipe Demo - Fades LEDs to Black then populates the new colour either bottom up or top down

https://user-images.githubusercontent.com/3640168/205511934-5211cd09-e835-44e0-9252-703d920e78c4.mp4

Shuffle In Demo - Shuffles in a new colour one pixel at a time

https://user-images.githubusercontent.com/3640168/205514276-fbe0be17-f728-4031-9431-8a6b12e9e94f.mp4



### Andy's Tree
![Tree](https://github.com/jfrmilner/Arduino-CheerLights/blob/master/Images/AMGoldschmidt_Tree2018_Tweet.jpg)




More details can be found here: http://jfrmilner.co.uk/christmasiotree
