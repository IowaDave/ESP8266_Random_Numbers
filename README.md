# True Random Numbers From an 8266?
by David "IowaDave" Sparks
December 2020

It may be possible to obtain truly random numbers from an ESP 8266 microcontroller.

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
uint32_t hwrand = *(volatile uint32_t *)0x3ff20344;
```

Note that the address might be different for different Expressif products. The ESP32's register could be located at 0x3FF75144, for example. It's probably best to use a predefined macro or a function call for the purpose.

A contributor to the Arduino forums, named "cossoft", identified that the Arduino compiler recognizes a macro (for 8266 devices) to do the same thing. [(2)](https://forum.arduino.cc/index.php?topic=592849.0) The macro is named "RANDOM_REG32" and it works like this: 

```
uint32_t hwrand = RANDOM_REG32;
```

The Expressif programming reference for the 8266, discussed in more detail below, documents yet a third, "official" way,  using a function call: ESP.random().

Here is a short Arduino IDE sketch to demonstrate the function call into the "hardware random number generator" of an 8266.

```
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(5000);
  Serial.println("\r\nSerial Started\r\n");
  // print ten random, 32-bit integers
  // in hexadecimal and decimal format
  for (int i=0; i < 10; i++) {
    uint32_t hwrand = ESP.random();
    Serial.printf("%08x = %10u\r\n", hwrand, hwrand);
  }
}

void loop() {
  // no loop code
}
```

The code produced the following output, for example:

```
Serial Started

67fe883a = 1744734266
29dd8127 =  702382375
422c3517 = 1110193431
c79cae39 = 3348934201
7ce64986 = 2095466886
01c144d7 =   29443287
919986ae = 2442757806
8777621e = 2272748062
158838c5 =  361248965
fb9deeb6 = 4221431478
```

### What is driving the hardware random number of an 8266?

The Expressif Programming Guide for 8266 documents two library calls for a hardware random number generator on the chip. [(3)](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/api-reference/system/system.html) The calls are: esp_random(), to fetch a single, 32-bit value; and esp_fill_random(), to fill a buffer with random bytes. 

Intriguingly, the reference for esp_fill_random() includes this, ahem, cryptic remark, "This function has the same restrictions regarding available entropy as esp_random()."  What are those restrictions? It doesn't say.

We get some clues, perhaps, from the corresponding reference for the ESP32: [(4)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html)

> ESP32 contains a hardware random number generator, values from it can be obtained using esp_random().

> When Wi-Fi or Bluetooth are enabled, numbers returned by hardware random number generator (RNG) can be considered true random numbers. Without Wi-Fi or Bluetooth enabled, hardware RNG is a pseudo-random number generator. At startup, ESP-IDF bootloader seeds the hardware RNG with entropy, but care must be taken when reading random values between the start of app_main and initialization of Wi-Fi or Bluetooth drivers.

I interpret this information to mean that the "hardware" aspect of the thing depends on the 8266's radios for its entropy, meaning, randomness-from-the-physical-world. It's purely a "pseudo" random number generator ("prng"), that is, a computational algorithm, without the radios. Even after being "seeded" from the radios, the prng algorithm is what actually produces (most of? maybe all of?) the values that show up at the address, 0x3ff20344.

### Is that good?

Yes and no. It is good in the sense that it sounds close to what other, modern computers do. The so-called hardware rng's built into Intel boxes and Raspberry Pis apply a similar principle. A hardware circuit generates some entropy, which then gets mixed into a prng. The prng winds up being the main source of random numbers for the system.

It is theoretically sound to use a high-quality prng for generating a series of random numbers, rather than using raw, physical entropy. We usually want random numbers that pass statistical tests. We want them to appear "independent and identically distributed." 

A high-quality prng can generate streams of billions of numbers that satisfy this desirable appearance. In other words, there is no way to tell that they are *not* random. Except... the values coming out of a prng are "deterministic", which means they depend on other, previous values. 

A really smart codebreaker can figure out the prng algorithm, if they get access to a long-enough series of the numbers it produces. There's math for that. After the codebreaker figures it out, then they can predict every value that follows. Which stinks, if you're hoping to use those pseudo-random numbers to encrypt something.

By contrast, bits of physical entropy (such as radioactive decay events), are truly independent of one another. Even the smartest codebreaker having the whole history of the entropy could never predict the next event. Trouble is, this kind of entropy might not have the uniformity of distribution that we desire in our random numbers. 

Combining the two concepts, as the 8266 does, means we may get "independence" from entropy, then we may get the desired distributional quality from the prng. 

It's not quite as good if the prng on the 8266 only gets benefit of entropy at startup but not afterward. And there's no physical entropy available if your sketch turns off the radio.

I have not been able to find an authoritative statement regarding whether the 8266 mixes entropy into its "hardware" rng continuously, rather than only at startup.

And keep in mind that the "hardware" rng does not actually expose the bits of entropy available from the radios.

### Is there a way to get actual, physical entropy from an 8266?

Yes. There might be. Maybe not from the radios. But quite possibly from the operation of the CPU, itself.

JV Roig, of Asia Pacific College, published an intriguing paper in 2018 about exploiting the variability of cpu run-times as a source of physical entropy for random number generation and cryptography. [(5)](http://research.jvroig.com/siderand/RoigJV_Crypto_FullPaper_ICIRSTM2018_2018-07-30.pdf)

The paper gives an interesting and easily understood critique of the so-called "hardware random number generators" that come built-into computer CPUs these days. The one on the 8266 would be subject to the same, doubtful remarks. See the paper for this worthwhile commentary.

We can easily do better, according to Roig. Give the CPU a long loop to execute. It can be as simple as adding the same two numbers over and over. Measure how long it takes. Repeat. Keep track of the measurements because they are likely to vary. The variation is unpredictable; Bingo! entropy.

It is a kind of side-channel attack on the cpu, for the purpose of extracting entropy from it. Roig proposes that the technique can succeed with any device that contains a cpu. He named it, "SideRand".

I playfully modified SideRand for the Arduino IDE, targeting an 8266-based ESP-01 module. And I think it works.

The trick was how to harvest the entropy and distill it into random numbers. Roig published several versions of his paper, in which he offered listings in C and Python. Alas, he stops at the point of having gathered a lot of different timing measurements, but does not go on to demonstrate the next step.

He does hint at the step, however. In one of the papers he mentions that the measurements "get hashed." Ah-hah! thought I. Feed the measurements to a hash algorithm.

The SHA256 algorithm enjoys a good reputation for digesting lengthy streams of bytes into 256-bit (32-byte) strings that appear random. It is claimed (and so far not refuted, to the best of my knowledge) that if more than 256 "bits of entropy" are in the stream of bytes entering the hash algorithm, then the resulting hash will also contain nearly 256 bits of entropy.

Remember, entropy is Mother Nature's random number generator. Not even the best codebreaker is going to hack Mother Nature. 

#### What does it mean?  
An 8266 can be a source of entropy taken directly from physical hardware if: 
1. Roig is right about scraping entropy off the side of a hard-working cpu, and 
2. I'm right about using a hash algorithm to distill that entropy into a number format.

### An Arduino IDE sketch for SideRand on an ESP8266
The Github repository that hosts this article also contains my SideRand sketch for the 8266. Here is a link to it: 

[https://github.com/IowaDave/ESP8266_Random_Numbers](https://github.com/IowaDave/ESP8266_Random_Numbers)

*Comment: this baby takes a long time to run on an 8266. Be patient.*

I modified Roig's algorithm in four ways.
1. Replace the timing methods in Roig with the micros() function of Arduino IDE.
2. Increase the number of samples taken, based on analysis of the variation in 8266 cpu runtimes as demonstrated by Roig. The increase seemed necessary to ensure more than 256 bits of entropy  feeding into the SHA256 hash algorithm.
3. Use a hash algorithm to digest the hardware entropy into numerical form.
4. Add a command to reset the 8266 watchdog timers frequently. Otherwise the chip resets itself and we never get anywhere.

### Should a hash algorithm be used as a random number generator?

That's another conversation for another time, outside the scope of this article. The gist of goes something like this:

* Hash algorithms in the SHA family produce outputs that for all practical purposes look random. Call that a "yes."
* Most of the so-called "hardware" random number generators feed their entropy through a hash in the normal course. Call that a "yes."
* A hash cannot *add* entropy that was not present in the stream of bytes fed into it. Care must be taken to gather enough entropy for the hash to digest. Call that a "maybe, it depends" on what comes before and goes into the hash.
* Gathering entropy for a hash takes time. It's a slow process. Usually we want random numbers faster than a purely entropy-driven process can obtain. Call that a "maybe, maybe not."

What does appear to work well is to use a high-quality pseudo-random number generator (PRNG) algorithm, but to "seed" it with entropy obtained from a physical-world source. The random-looking bytes that emerge from SideRand and SHA256 running on an 8266 could be candidates for seeding such a PRNG.

Of course, so might the entropy, derived from the radios, available to the 8266's own generator. And this combination does appear to work well from a statistical perspective. At least one enthusiast, Dr. Abhishek Ghosh, wrote earlier this year that the "hardware" random number generator on the 8266's sister chip, the ESP32, has been shown to pass a very difficult set of statistical tests when running with the radios enabled. [(6)](https://thecustomizewindows.com/2020/05/esp32-as-hardware-random-number-generator/)

### But... Is It Cryptographically Secure?
Ah, that question always comes up. My answer is, Who Knows? Is anything safe from the Codebreaker? 

### Summary

If you want a large number of statistically high-quality random numbers really fast, consider using the "hardware" random number generator of an 8266. Be sure to turn on the radios in your setup procedure, for example this way:

```
#include <ESP8266WiFi.h>

void setup() {

    WiFi.mode(WIFI_STA)

}
```

The SideRand => SHA256 technique I toyed with produces numbers that look random to me. They come too slowly for any really robust statistical test, alas. It means I cannot express an opinion about their distribution. However, it does seem to me that they might be "independent", in the sense of being unpredictable. If that is true, then they could be used as seeds for a PRNG.

Roig professes a conviction that the entropy captured by the SideRand algorithm may afford a source of random numbers strong enough to be considered cryptographically secure. 

***
Footnotes

(1) "ESP8266 Hardware Random Number Generator Via 0x3FF20E44". [https://twitter.com/ESP8266/status/692469830834855936](https://twitter.com/ESP8266/status/692469830834855936). Accessed 14Dec2020. Note: click on the image of the text to view a "photo" of the complete posting.

(2) "Any details of the esp8266 hardware random number generator?". [https://forum.arduino.cc/index.php?topic=592849.0](https://forum.arduino.cc/index.php?topic=592849.0). Accessed 14Dec2020. Note: scroll down to comment #10.

(3) System &mdash; ESP8266 RTOS SDK Programming Guide. [https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/api-reference/system/system.html](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/api-reference/system/system.html). Accessed 14Dec2020.

(4) Miscellaneous System APIs - ESP32 - — ESP-IDF Programming. [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html). Accessed 14Dec2020.

(5) Roig, JV. "Stronger Cryptography For Every Device, Everywhere: A Side-ChannelBased Approach to Collecting Virtually Unlimited Entropy In Any CPU". 2018. [http://research.jvroig.com/siderand/RoigJV_Crypto_FullPaper_ICIRSTM2018_2018-07-30.pdf](http://research.jvroig.com/siderand/RoigJV_Crypto_FullPaper_ICIRSTM2018_2018-07-30.pdf). Accessed 14Dec2020.

(6) Ghosh, Abhishek. "ESP32 as Hardware Random Number Generator". 2020. [https://thecustomizewindows.com/2020/05/esp32-as-hardware-random-number-generator/](https://thecustomizewindows.com/2020/05/esp32-as-hardware-random-number-generator/). Accessed 14Dec2020.