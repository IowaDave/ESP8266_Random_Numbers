/****************************************
 * Gather entropy directly from the CPU *
 * of an AVR MPU. Condense the entropy  *
 * into a hash using SHA256 algorithm.  *
 * Output the hash via Serial.          *
 *                                      *
 * External library usage:              *
 * add crypto library by Rhys Weatherly *
 * to the Arudino IDE.                  *
 * From it, #include the SHA256 header. *
 *                                      *
 * SideRand code adapted from JV Roig,  *
 * "SideRand: A Heuristic and Prototype *
 * of a Side-Channel-Based              *
 * Cryptographically Secure Random      *
 * Seeder Designed to Be Platform-      *
 * and Architecture-Agnostic "          *
 * published April 2018 in Computer     *
 * Science, available from Cornell      *
 * University online at url:            *
 * https://arxiv.org/abs/1804.02904     *
 ****************************************/

// crypto library by Rhys Weatherly
#include <SHA256.h>

// watchdog timer library
#include <avr/wdt.h>

// standard C library, for sprintf()
#include <stdio.h>;

// Global constants
// number of timing samples to collect, per round
#define SAMPLE_SIZE 100
// number of rounds to obtain 1000 samples
#define SAMPLE_ROUNDS (1000 / SAMPLE_SIZE)

// object to construct the hash
SHA256 myHash;
// buffer to receive the hash from the object
long hashBytes[8];
// array to collect the timing samples
// note: memory constrains the size of this array
long timingSamples[SAMPLE_SIZE];
// buffer to receive formatted string of hash bytes
char formatString[10];

// other variables
 int k=0;

// prototype declaration of the sideRand() function
void sideRand();

void setup() {

  // ensure formatString buffer terminates with a null
  formatString[9] = '\0';
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("Serial started");
  Serial.println();
}

void loop() {
  // Start a new hash
  Serial.println();
  Serial.println("New hash:");
  // clear the hash and prepare it for new input
  myHash.clear();

  // perform the siderand subroutine enough times
  // to accumulate 1000 timing samples
for (k = 0; k < SAMPLE_ROUNDS; k++) {
  // fill the timingSamples buffer with timing samples
  sideRand();
  // feed the timing samples buffer into the hash
  myHash.update(timingSamples, sizeof(long) * SAMPLE_SIZE);
}

  // finalize the hash in the hash bytes output buffer
  myHash.finalize(hashBytes, sizeof(long) * 8);
  // terminate the line of dots that indicate progress
  Serial.println();
  // output the hash from the array, via Serial,
  // as a series of strings of 4-byte integers
  for (k=0; k<8; k++)
  {
    sprintf(formatString, "%08lX", hashBytes[k]);
    Serial.print(formatString);
  }
  // terminate the string
  Serial.println();
  // The SHA256 portion of the loop is complete.
    
  // Yield allows the MPU to perform housekeeping,
  // or to process other Scheduled threads, 
  // if that functionality has been implemented.
  // Otherwise, it does nothing
  yield();
}

void sideRand() {

// number of additions to perform per sample
#define SCALE 5000000

// variables used by SideRand algorithm
 long i=0;
 long j=0;
 const long val1 = 2585566630;
 const long val2 = 576722363;
 long total;
 long runTime;
 long begin;
 long end;

      // fill a buffer with timing samples
  for(i=0; i<SAMPLE_SIZE; i++)
  {
    begin = micros();
    for(j=0; j<SCALE; j++)
    {
      total = 0;
      total = val1 + val2;
    }
    end = micros();
    timingSamples[i] = end - begin;
    
    // IMPORTANT: reset the AVR watchdog timer
    // so that this loop does not reset the AVR!
   wdt_reset();
  }
    // put a dot out to indicate progress
    Serial.print(".");

  // This SideRand round is complete.
  // The timing samples are in the timingSamples array.

}
