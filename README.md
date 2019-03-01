# Voice modulator

Simple voice modulator written in c.

# Functionality

Voice modulator takes system default audio input stream and modulates sine frequency with sin_freq (default: 30hz)
on it to let you sound like a Dalek.

# Dependencys

You need to install portaudio.
* Arch: pacman -S portaudio
* Ubuntu: apt-get install portaudio19-dev

# Installation

Navigate into voice_modulator
```bash
mkdir build
cd build
cmake ..
make
```
