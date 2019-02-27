# ha-mon 0.8

**ha-mon** is a *generative music instrument* for **Android**, written entirely in C. It consists of a native activity and primarily uses **OpenGL ES 2** and **OpenSL ES** for drawing graphics and playing sounds.

Audio playback is based around a polyphonic sampler that implements a buffer loop callback to stream sample data. Playback and recording of note event data piggybacks upon OpenSL ES's high priority thread to provide robust and reliable sample-accurate timing. 

The human ear is particularly sensitive to disparities between the timing of sonic events, especially those of a rhythmic, repetitive nature. Such precise timing was not possible even when using native timer functions.

This project would also be a good starting point for those who want to build completely native Android software using OpenGL ES 2 and OpenSL ES. Please note that this repository contains no sound and graphics asset files. A placeholder set of media assets is planned for future versions.

*Note:* as an avid Japanese learner, some of the comments are in Japanese. I've made every effort to provide an English equivelent, but some Japanese only comments might still exist throughout the source code.

# INSTALLATION

To clone:

        git clone git@github.com:kapsy/hamon.git

To setup assets:

#### ECLIPSE SETUP:

* Select *File > Import...*, then *General > Existing Project into Workspace* and then *Next*.
* Browse to the directory containing the cloned project.
* Under *Projects* select *hamon*, and then *Finish* to start the import.

If any error messages are encountered you may have to select *Project > Clean* and then select *Clean all projects*. 

# OUTLINE

There are two modes of play with **ha-mon**. The first mode, *auto mode*, is set upon startup, and is where note events are automatically triggered at random intervals. The second, interactive mode, is set as soon as the player touches the play space. If all note events are exhausted, *auto mode* automatically starts after a short while.

In either mode, all note events are recorded and looped back over a short period. Once the currently recording loop is finished, all events from that point are recorded on the next available loop. There are slight time differences in the loop lengths, meaning that the set of note events from any loop never quite play in sync with each other. This subtlty creates complex, never-repeating, ever evolving musical patterns. 

All recorded note events also have a limited life span to make way for new ones, meaning the musical pallette is constantly renewing itself - a total contrast to linear, recorded music. 

At any one time, the sound that the note events trigger entirely depends on what *sound color* is currently selected. The *sound color*, represented pictorally by the color of the ripple shapes and background, ensures that all notes played are part of a coherent musical scale. In *auto mode* the selected *sound color* is cycled over a random period, in *interactive mode* it can be changed at any time by the player.

The player has a limited amout of *ammunition* that can be used to record note events. When all *ammuntion* is exhausted, a small, non-event sound and animation is played on each tap. The *ammunition* always regenerates at a slow rate. This is to prevent excessive notes being recorded - leading to the undesirable side-effects of voice overload and drops in framerate.
