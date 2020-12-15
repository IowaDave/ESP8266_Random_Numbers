# True Random Numbers From an 8266?
by David "IowaDave" Sparks
December 2020

It may be possible to obtain truly random numbers from an ESP 8266 (or ESP32) microcontroller.

In fact, there are at least two ways to go about it:

1. Use the "hardware random number generator" built into the device.
2. Exploit potentially random variation in the time it takes the CPU to complete a task.

It's remarkably easy and fun!  All you need to follow this project is a computer running the Arduino IDE and an 8266 development board &mdash; even the tiny ESP-01 module will do.

### Get serious for a moment
Readers should take note that I disclaim and make no effort to prove that these methods produce truly random numbers. Take what's here in a playful spirit, have fun, maybe learn something, and that's all.

Keep in mind that honest-to-gosh, proven, commercial-grade hardware random number generators sell for thousands of dollars. Don't expect a five-dollar gizmo to be as good as that.

Now, let the fun begin!

### Built-in Hardware Random Number Generator

A Twitter contributor named @projectgus posted the news that seemingly random, 32-bit unsigned integers were available at an address in the device hardware, specifically 0x3ff20e44. [(1)](https://twitter.com/ESP8266/status/692469830834855936) He demonstrated accessing the address directly with this code: 

```
uint32_t hwrandom = *(volatile uint32_t *)0x3ff20344;
```

A contributor to the Arduino forums, named "cossoft", identified that the Arduino libraries for the 8266 provide a macro that does the same thing. [(2)](https://forum.arduino.cc/index.php?topic=592849.0) The macro is named "RANDOM_REG32" and it works like this: 

```
uint32_t hwrandom = RANDOM_REG32;
```

Here is a short Arduino IDE sketch to demonstrate the "hardware random number generator" of an 8266.

```
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(5000);
  Serial.println("\r\nSerial Started\r\n");
  // print ten random, 32-bit integers
  // in hexadecimal and decimal format
  for (int i=0; i < 10; i++) {
    uint32_t hwrand = RANDOM_REG32;
    Serial.printf("%08x = %10u\r\n", hwrand, hwrand);
  }
}

void loop() {
  // no loop code
}
```

For example, the code produced the following output:

<code>
Serial Started

b4a36d3a = 3030609210
3ea82778 = 1051207544
6c43a6f5 = 1816372981
43254160 = 1126515040
713b8ed7 = 1899728599
be95927b = 3197473403
7cefeef2 = 2096099058
660ce9eb = 1712122347
39b9258a =  968435082
d907b1cc = 3641160140
</code>

***
Footnotes

(1) "ESP8266 Hardware Random Number Generator Via 0x3FF20E44", [https://twitter.com/ESP8266/status/692469830834855936](https://twitter.com/ESP8266/status/692469830834855936), accessed 14Dec2020. Note: click on the image of the text to view a "photo" of the complete posting.

(2) "Any details of the esp8266 hardware random number generator?", [https://forum.arduino.cc/index.php?topic=592849.0](https://forum.arduino.cc/index.php?topic=592849.0), accessed 14Dec2020. Note: scroll down to comment #10.
