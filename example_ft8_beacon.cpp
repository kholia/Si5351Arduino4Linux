// (c) Rafal Rozestwinski (callsign: SO2A), https://github.com/webft8/Si5351Arduino4Linux
// (c) Dhiru Kholia (callsign: VU3CER), https://github.com/kholia
//
// Example usage, documentation: https://github.com/webft8/Si5351Arduino4Linux
// Dependencies: sudo apt-get install libi2c-dev
// Link with -li2c
// Basic build command: g++ -li2c example.cpp si5351.cpp -o ./si5351_cli

#include <cstdint>
#include <inttypes.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <time.h>
#include <stdlib.h>
#include "si5351.h"
#include "ft8/FT8.h"

#include <unistd.h>
#include <iostream>
#include <errno.h>

uint64_t frequency = 28075000ULL * 100ULL; // CHANGE THIS PLEASE
uint8_t tones[255];

int toneDelay;
int symbolCount;
int toneSpacing;

#define FT8_TONE_SPACING 625ULL  // ~6.25 Hz
#define FT8_DELAY 152     // Delay value for FT8 (ms) - ATTENTION: Original value was 159!
char message[] = "VU3CER VU3FOE MK68";

// Pins
#define SW1 18  // For https://github.com/kholia/HF-PA-v10

int encoder(char *message, uint8_t *tones, int is_ft4);

void sleep_func(uint32_t ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = ms % 1000 * 1000000;

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

int main(int argc, char** argv) {
    int i2c_device_number = 0;
    int calibration = 0;
    printf("Using i2c device number: %i\n", i2c_device_number); fflush(stdout);
    printf("Setting SI5351 calibration to %i\n", calibration); fflush(stdout);
    printf("Base frequency is %" PRIu64 "\n", frequency); fflush(stdout);
    Si5351 si5351(i2c_device_number);
    bool i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 60000);
    // bool i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 26000000, 0); // 26 MHz TCXO
    if(!i2c_found) {
        printf("Device not found on I2C bus!\n");
        exit(1);
    }
    si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

    // Prep for FT8
    symbolCount = FT8_SYMBOL_COUNT;
    toneSpacing = FT8_TONE_SPACING;
    toneDelay = FT8_DELAY;
    ftx_encode(message, tones, false);

    printf("FT8_SYMBOL_COUNT %d\n", symbolCount);
    for (int k = 0; k < symbolCount; k++) {
        printf("%d ", tones[k]);
    }
    printf("\n");

    time_t now;
    struct tm *tm;

    while (1) {
        uint8_t i;

        now = time(0);
        if ((tm = localtime (&now)) == NULL) {
            printf("Error extracting time stuff\n");
            return 1;
        }

	if (tm->tm_sec % 15 == 0) {
            printf("[%d] TX ON!\n", tm->tm_sec);
            si5351.output_enable(SI5351_CLK0, 1);
            for (i = 0; i < symbolCount; i++) {
                uint64_t f = frequency + (tones[i] * toneSpacing);
                si5351.set_freq(f, SI5351_CLK0);
                sleep_func(toneDelay);
            }
            now = time(0);
            if ((tm = localtime (&now)) == NULL) {
                printf("Error extracting time stuff\n");
                return 1;
            }
            printf("[%d] TX OFF!\n", tm->tm_sec);
            si5351.output_enable(SI5351_CLK0, 0);
	}

	usleep(100);
    }

    return 0;
}
