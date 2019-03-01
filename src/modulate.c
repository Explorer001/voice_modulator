#include <stdio.h>
#include <math.h>
#include <portaudio.h>
#include <stdbool.h>
#include <signal.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 64

struct paData
{
    int left_phase;
    int right_phase;
    float *sin;
    size_t sin_len;
};

//sin freq in hz
static float sin_freq = 30.0f;

//defines change in signal
static float add = 0.0f;

//lets modulating loop run until changed to false
static bool running = true;


void intHandler(int arg)
{
    running = false;
}

//callback for audio stream -> takes input and modulates sin function on it
static int pa_callback( const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
    struct paData *data = (struct paData *) userData;
    float *out = (float *) outputBuffer;
    float *in = (float *) inputBuffer;
    unsigned long i;

    //set output stream
    for (i = 0; i < framesPerBuffer; i++)
    {
        *out++ = *in++ * data->sin[data->left_phase];
        *out++ = *in++ * data->sin[data->right_phase];
        data->left_phase += 1;
        data->right_phase += 1;
        if (data->left_phase >= data->sin_len || data->left_phase >= data->sin_len)
        {
            data->left_phase = data->right_phase = 0;
        }
    }

    return paContinue;
}

// SAMPLE_RATE / sin_freq = table size
size_t get_sin_size()
{
    return (size_t) SAMPLE_RATE / sin_freq;
}

int main(void)
{
    PaStreamParameters outputParameters;
    PaStreamParameters inputParameters;
    PaStream *stream;
    PaError err;
    struct paData data;
    PaDeviceIndex device_count;

    //initializing paData struct
    data.left_phase = data.right_phase = 0;
    //init sin lookup table
    size_t sin_size = get_sin_size();
    float sine[sin_size];
    for (int i = 0; i < sin_size; i++)
    {
        sine[i] = (float) sin((double) i / (double) sin_size * M_PI * 2.);
    }
    data.sin = sine;
    data.sin_len = sin_size;

    //set signal handler to catch SIGINT
    signal(SIGINT, intHandler);

    //initialize portaudio
    printf("Initializing portaudio...\n");
    err = Pa_Initialize();
    if (err != paNoError)
    {
        printf("Initialization failed: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return -1;
    }

    device_count = Pa_GetDeviceCount();
    printf("Found %d devices.\n", device_count);

    printf("Initialized portaudio.\n");
    //open default output and set output parameters
    printf("Get output device...\n");
    outputParameters.device = Pa_GetDefaultOutputDevice();
    printf("Using device %d\n", outputParameters.device);
    if (outputParameters.device == paNoDevice)
    {
        printf("Getting device failed!\n");
        Pa_Terminate();
        return -1;
    }
    outputParameters.channelCount = 2; /* stereo */
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    //open default input device
    printf("Get input device...\n");
    inputParameters.device = Pa_GetDefaultInputDevice();
    printf("Using device %d\n", outputParameters.device);
    if (outputParameters.device == paNoDevice)
    {
        printf("Getting device failed!\n");
        Pa_Terminate();
        return -1;
    }
    inputParameters.channelCount = 2;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    //open output stream
    printf("Opening output stream...\n");
    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff, /* no need to clip if samples not out of range */
        pa_callback,
        &data);
    
    if (err != paNoError)
    {
        printf("Opening stream failed!\n");
        Pa_Terminate();
        return -1;
    }
    printf("Opened stream.\n");

    //starting stream
    printf("Starting stream...\n");
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        printf("Starting stream failed!\n");
        Pa_Terminate();
        return -1;
    }

    while (running)
    {}

    //stop stream
    printf("Stopping stream...\n");
    err = Pa_StopStream(stream);
    if (err != paNoError)
    {
        printf("Stopping stream failed!\n");
        Pa_Terminate();
        return -1;
    }

    Pa_Terminate();
    return 0;
}