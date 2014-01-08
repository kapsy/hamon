# ha-mon 0.8

**ha-mon** 

Please note that **bass bender** requires *libpd* in order to run the *Pure Data* patches. If you're unfamiliar with what *libpd* and *Pure Data* are, more info can be found at <http://github.com/libpd> and <http://puredata.info/>.

### INSTALLATION

        git clone git@github.com:kapsy/bass-bender.git

PD for Android install:

        git clone git://github.com/libpd/pd-for-android.git
        cd pd-for-android
        git submodule init
        git submodule update

#### ECLIPSE SETUP:

* From the main menu bar, select *File > Import...*, and the *Import wizard* opens.
* Select *General > Existing Project into Workspace* and click *Next*.
* Browse to locate the directory containing the previously cloned projects.
* Under *Projects* select *bass-bender*, *PDCore*, *AndroidMidi*.
* Click *Finish* to start the import.

If any error messages are encountered you may have to select *Project > Clean* and then select *Clean all projects*. 

Depending on where you've saved *pd-for-android* you may need to modify the path to *PDCore*. Right click on the project and select *Properties*, then *Android*. On the right there's a dialog to add a reference to the *PDCore* library.

### USAGE

The application itself plays a continous loop of FM-synthesized bass notes in a generative fashion. When the user drags one finger across the screen, the X axis applies pitch bending to the notes, and the Y axis changes the pitch of the fundamental FM frequency. 

When two fingers are used in a pinch-zoom fashion, the distance between the first and second finger creates a sort of fader, which controls the amplitude of the FM signal, creating a growling sound. 

All touch movements, represented by the green dots, are recorded over a 25 second period then continuously played back, a state that is represented by blue dots. This further adds to the genererative nature of the sounds produced, creating ever changing sound textures that do not become repetitive to the listener.

Any time the screen is touched in a playback state, the current recording is overwritten and the loop is started again. The position in the touch recording/playback loop is represented by the thin bar at the bottom of the screen.

It is recommended to install on devices runnning Android 4.1 and above due to shorter audio playback latency. Also, as the sounds are of a low frequency, they can only be heard properly on full-range speakers or headphones - the built in speakers found in most smartphones and tablets are not recommended. 

### VERSION HISTORY

#### 1.1
* Updated splash screen design.
* Removed unused image files for a smaller apk.
* Tidied project and added .gitignore.

#### 1.0
* Initial release.
