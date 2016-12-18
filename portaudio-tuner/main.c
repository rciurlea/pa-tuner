//
//  main.c
//  portaudio-tuner
//
//  Created by Radu Ciurlea on 09/12/16.
//  Copyright © 2016 Radu Ciurlea. All rights reserved.
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#define SAMPLE_RATE (44100)
#define FRAMES (512)
#define MAX_TAU (512)
#define WINDOW (512)

int main(int argc, const char * argv[]) {
    PaError err;
    float buffer[FRAMES * 4];
    float d[MAX_TAU];
    float r[MAX_TAU];
    
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        exit(1);
    }
    
    PaStream *stream;
    err = Pa_OpenDefaultStream( &stream,
                               2,          /* no input channels */
                               0,          /* stereo output */
                               paFloat32,  /* 32 bit floating point output */
                               SAMPLE_RATE,
                               FRAMES,        /* frames per buffer, i.e. the number
                                            of sample frames that PortAudio will
                                            request from the callback. Many apps
                                            may want to use
                                            paFramesPerBufferUnspecified, which
                                            tells PortAudio to pick the best,
                                            possibly changing, buffer size.*/
                               NULL,        /* this is your callback function */
                               NULL ); /*This is a pointer that will be passed to
                                         your callback*/
    if(err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        exit(1);
    }
    
    Pa_StartStream(stream);
    
    for (int i = 0; 1; i++) {
        // shift buffer to the left
        for (int j = 1; j < 2 * FRAMES; j += 2) {
            buffer[j] = buffer[j+2*FRAMES];
        }
        // read some audio data...
        err = Pa_ReadStream(stream, &buffer[FRAMES] , FRAMES);
        if(err != paNoError) {
            printf("PortAudio read error: %s\n", Pa_GetErrorText(err));
        }

        // yin algorithm
        // 1 - compute difference function
        for (int j = 0; j < MAX_TAU; j++) {
            float s = 0;
            for (int k = 0; k < WINDOW; k++) {
                // s += (data[j] - data[j+i]) * (data[j] - data[j+i]);
                s += (buffer[k*2+1] - buffer[(k+j)*2+1]) * (buffer[k*2+1] - buffer[(k+j)*2+1]);
            }
            d[j] = s;
        }
        
        // 2 - normalized difference function
        r[0] = 1;
        float sum = d[0];
        for (int j = 1; j < MAX_TAU; j++) {
            sum += d[j];
            r[j] = d[j] / ((1.0 / j) * sum);
        }
        
        // 3 - look for minimum and print frequency
        for (int j = 1; j < MAX_TAU - 1; j++) {
            if (r[j] < 0.1) {
                if (r[j] < r[j-1] && r[j] < r[j+1]) {
                    printf("frequency: %fHz\n", (float)44100 / j);
                    break;
                }
            }
        }
    }
    
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    

    err = Pa_Terminate();
    if(err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }
    
    printf("heh, success!\n");
    return 0;
}
