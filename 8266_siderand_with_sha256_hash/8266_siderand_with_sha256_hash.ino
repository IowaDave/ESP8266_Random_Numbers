/****************************************
 * Gather entropy directly from the CPU *
 * of the 8266. Condense the entropy    *
 * into a hash using SHA256 algorithm.  *
 * Output the hash via Serial.          *
 *                                      *
 * External library usage:              *
 * add crypto library by Rhys Weatherly *
 * to the Arudino IDE.                  *
 * From it, #include the SHA256 header. *
 *                                      *
 * SideRand code adapted from JV Roig:  *
 * http://research.jvroig.com/siderand/ *
 * RoigJV_Crypto_FullPaper_ICIRSTM2018_ *
 * 2018-07-30.pdf                       *
 ****************************************/
// from crypto library by Rhys Weatherly
#include <SHA256.h>

// number of timing samples to collect
#define SAMPLES 1000
// number of additions to perform per sample
#define SCALE 5000000

// object to construct the hash
SHA256 myHash;
// buffer to receive the hash from the object
int hashBytes[8];
// array to collect the timing samples
int timingSamples[SAMPLES];

// variables used by SideRand algorithm
 int i=0;
 int j=0;
 int val1 = 2585566630;
 int val2 = 576722363;
 int total;
 int runTime;
 int begin;
 int end;

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println();
  Serial.println("Serial started");
  Serial.println();
}

void loop() {
  // SideRand portion of the loop
  // fill a buffer with timing samples
  for(i=0; i<SAMPLES; i++)
  {
    begin = micros();
    for(j=0; j<SCALE; j++)
    {
      total = 0;
      total = val1 + val2;
    }
    end = micros();
    timingSamples[i] = end - begin;
    // put a dot out every 100 passes
    if ((i % 100) == 0) {Serial.print(".");}
    
    // IMPORTANT: reset the 8266 watchdog timers
    ESP.wdtFeed();
  }
  // The SideRand portion is complete.
  // All of the timing samples are in the array.
  // Begin the SHA256 portion of the loop.
  Serial.println();
  Serial.println("New hash:");
  // clear the hash and prepare it for new input
  myHash.clear();
  // send the timing samples array into the hash
  myHash.update(timingSamples, sizeof(int) * SAMPLES);
  // finalize the hash, transfer it to the buffer
  myHash.finalize(hashBytes, sizeof(int) * 8);
  // output the hash as 8, 32-bit unisgned integers
  for (i=0; i<8; i++)
  {
    Serial.printf("%08x\r\n", hashBytes[i]);
  }
  // The SHA256 portion of the loop is complete.
  // Yield allows 8266 to perform housekeeping.
  yield();
}
