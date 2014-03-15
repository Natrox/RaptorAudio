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

Streaming or not?
-------
OGG is only support with StreamingSoundObject. StreamingSoundObject does not necessarily mean that the whole file is streamed from disk, it merely means that not all of the sound data will be available at all times. Note that it is possible to use StreamingSoundObject with pre-loaded memory. As such, **MemorySoundObject does not support OGG!**

Usage
=======
Using the basic functionality of RaptorAudio is very simple. First, the libraries must be linked and the headers must be included. Then, the audio engine has to be initialized. 

```cpp
#include "RaptorAudio.h"

using namespace Raptor::Audio;

...
// Example
SoundMixer::InitializeMixer( sampleRate, bufferSize, SoundMixerBufferingModes::BUFFERING_BLOCKS, SoundMixerProfiles::SOUND_MIXER_SPEAKERS );
```

From here on, the user may create sounds;

```cpp
// For streaming audio (from file).
StreamingSoundObject* sound = new StreamingSoundObject( "Space Faring.ogg" );

// For loaded audio (from file). MemorySoundObject does not support OGG!
MemorySoundObject* sound = new StreamingSoundObject( "Space Faring.wav" );

// For audio from memory.
StreamingSoundObject* sound = new StreamingSoundObject( ptr, AudioOrigins::AUDIO_ORIGIN_OPENMEMORY, length );
MemorySoundObject* sound = new MemorySoundObject( ptr, AudioOrigins::AUDIO_ORIGIN_OPENMEMORY_POINT, length );
```

If the user now wishes to just play the sound (fire and forget);

```cpp
SoundMixer::GetMixer()->PlaySoundObject( sound );
```

However, often at times, you may want to control the sound as it is playing;

```cpp
SoundObjectProperties props = SoundMixer::GetMixer()->CreateProperties( sound );

// Set sound to loop
props->SetLooping( true );

SoundMixer::GetMixer()->PlaySoundObject( props );

....

// Somewhere else
SoundMixer::GetMixer()->Stop( props );
```

Note that it is perfectly safe to use the 'props' object anywhere. It is garbage-collected, so it exists for as long as you keep it, and the sound mixer will safely ignore it if it is not relevant anymore.

Shared properties
-----------------
RaptorAudio provides a 'SharedProperties' object. This object can be shared between sounds (or used individually), and contains variables that govern the pitch, volume and DSP chain of sounds;

```cpp
SharedProperties sprops = CreateSharedProperties();
sprops->sp_Volume = 0.5;

// Link to a SoundObjectProperties
SoundObjectProperties props = SoundMixer::GetMixer()->CreateProperties( sound, sprops );
```
