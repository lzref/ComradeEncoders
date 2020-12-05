# Installation instructions

## MacOS
Find Ableton app in your Applications directory (for example, it can be `/Applications/Ableton Live 10 Suite.app`).
Right-click on it and choose Show Package contents. Then go to `Contents/App-Resources/MIDI Remote Scripts/`.
This is where you need to copy the directory (ComradeEncoders) that contains this README.md file.

Then open Ableton Live and go to Live => Preferences => MIDI. ComradeEncoders should show up in the dropdown
list for the control surfaces. Choose Maple as Input and Output.

That's it! Ableton should now be able to talk to ComradeEncoders controller.

If there's an error or something goes wrong, it may appear in this log file: `/Users/{your_username}/Library/Preferences/Ableton/Live {version}/Log.txt`

## Windows

Copy the directory that contains this README.md file (ComradeEncoders) here:

```C:\Program Files\Ableton\Live {version}\Resources\MIDI Remote Scripts\```

Then open Ableton Live and go to Options => Preferences => MIDI. ComradeEncoders should show up in the dropdown
list for the control surfaces. Choose Maple as Input and Output.

That's it! Ableton should now be able to talk to ComradeEncoders controller.

If there's an error or something goes wrong, it may appear in this log file:

`C:\Documents and Settings\{your_username}\Application Data\Ableton\Live {version}\Preferences\Log.txt`
