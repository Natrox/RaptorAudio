RaptorAudio
===========

RaptorAudio is a simple software audio engine with support for WAV and OGG files. It currently only runs on Windows. 

Current Issues
--------------

WAV files with metadata are not supported at this point in time. Please use OGG if you want to retain metadata.

Acknowledgements
--------------
I would like to thank the following people;

Nils Desle, my teacher at NHTV IGAD, for teaching me about audio programming in his audio course and providing feedback.

<a href="http://www.nothings.org/">Sean Barrett</a>, for providing stb_vorbis, which is used for OGG decompression in RaptorAudio.

Features
--------

RaptorAudio has basic mono or stereo sound playback, in both 2D and 3D. Sounds can be played from memory, or they can be streamed from disk. The engine also features a way to easily change sound parameters on-the-fly, using garbage-collected sound property objects. The engine has support for common WAV bitrates; 8-bit unsigned, 16-bit signed, 24-bit signed, 32-bit float.

Included as well is a DSP system; this system allows practically any operation on the playing sound data. DSP effects such as a simple LowPass/HighPass, Stereo Enhancer and Echo Enhancer are included, but users may create their own effects.

Mixing in RaptorAudio is done using a form of brick-wall compression. There are two possible outputs for the mixer. One is a block-buffering system, which is the canonical way to send data to the audio device. Additionally, there is also support for a ringbuffer, which has a bit more latency, but is a lot more stable on slow devices.

Feature sheet:
- Support for WAV (8bu/16bs/24bs/32bf) and OGG.
- Fully multithreaded.
- Support for the streaming of sounds.
- 3D sound and stereo sound in 3D (using a mono to stereo interpolation).
- Full DSP chaining pipeline with user customizable variables.
- Recyclable and shareable properties for 3D and DSP.
- Mixer profiles for both speakers and headphones setups.
- Multiple types of output buffers (block buffer, ring buffer).

Usage
=======
Using the basic functionality of RaptorAudio is very simple. First, the libraries must be linked and the headers must be included. Then, the audio engine has to be initialized. 

```cpp
#include "RaptorAudio.h"

using namespace Raptor::Audio;

...
// Example
SoundMixer::InitializeMixer( sampleRate, bufferSize, SoundMixerBufferingModes::BUFFERING_BLOCKS, SoundMixerProfiles::SOUND_MIXER_SPEAKERS );
