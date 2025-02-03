#include <iostream>
#include <fstream>
#include <alsa/asoundlib.h>
#include <termios.h>
#include <unistd.h>

#define PCM_DEVICE "default"
#define SAMPLE_RATE 44100
#define CHANNELS 1
#define BUFFER_SIZE 4096 // Buffer size in frames

// Function to capture key press
bool keyPressed() {
    struct termios oldt, newt;
    int ch;
    bool ret_val = false;

    tcgetattr(STDIN_FILENO, &oldt); // Get the terminal settings
    newt = oldt;
    newt.c_lflag &= ~ICANON; // Disable canonical mode
    newt.c_lflag &= ~ECHO; // Disable echo
    newt.c_cc[VMIN] = 1; // Minimum number of characters read
    newt.c_cc[VTIME] = 0; // No timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Set the new terminal settings

    ch = getchar();
    if (ch != EOF) {
        ret_val = true;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore terminal settings
    return ret_val;
}

void recordAudio(const std::string &filename);

int main() {
    std::string filename;

    std::cout << "Press Enter to start recording...\n";
    while (!keyPressed()) {} // Wait until Enter is pressed to start recording

    std::cout << "Enter filename for recording (e.g., output.wav): ";
    std::cin >> filename; // Get the filename after Enter is pressed
    
    std::cout << "Recording... Press Enter again to stop.\n";
    
    // Start recording
    recordAudio(filename);

    std::cout << "Recording finished. File saved as: " << filename << '\n';
    return 0;
}

void recordAudio(const std::string &filename) {
    snd_pcm_t *pcmHandle;
    snd_pcm_hw_params_t *params;

    // Open PCM device for recording
    if (snd_pcm_open(&pcmHandle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        std::cerr << "Error: Cannot open PCM device.\n";
        return;
    }

    // Allocate hardware parameters object
    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(pcmHandle, params);

    // Set hardware parameters
    snd_pcm_hw_params_set_access(pcmHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcmHandle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcmHandle, params, CHANNELS);
    snd_pcm_hw_params_set_rate(pcmHandle, params, SAMPLE_RATE, 0);

    // Apply hardware parameters
    if (snd_pcm_hw_params(pcmHandle, params) < 0) {
        std::cerr << "Error: Cannot set hardware parameters.\n";
        snd_pcm_hw_params_free(params);
        snd_pcm_close(pcmHandle);
        return;
    }

    snd_pcm_hw_params_free(params);

    // Open output file for saving raw audio
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Cannot open file for writing.\n";
        snd_pcm_close(pcmHandle);
        return;
    }

    // Buffer to hold audio data
    int16_t buffer[BUFFER_SIZE];

    // Start recording until Enter is pressed again
    bool recording = true;
    while (recording) {
        if (keyPressed()) {  // Check if Enter is pressed to stop recording
            recording = false;
        } else {
            int frames = snd_pcm_readi(pcmHandle, buffer, BUFFER_SIZE);
            if (frames < 0) {
                std::cerr << "Error: Failed to read from PCM device.\n";
                break;
            }
            outFile.write(reinterpret_cast<char *>(buffer), frames * CHANNELS * sizeof(int16_t));
        }
    }

    outFile.close();
    snd_pcm_close(pcmHandle);
}
