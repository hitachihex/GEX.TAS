# GEX.TAS
TAS Tools for the game GEX for the PC.

To use the tool, run Gex.TAS.Injector after running the game executable, make sure Gex.TAS.dll is in the same directory as the injector executable. You will need to run the injector (and possibly the game) as administrator.

You will need to manually create the input file in the directory where the game executable is (it looks for Gex.rec).

Hotkeys aren't configurable atm (sorry about that, I just get lazy and am used to this schema)

Hotkeys are basic:

F1 - To pause\start framestep
Numpad 9 - Unpause (the same key can't be used to unpause because of how the game loop is hooked and the API function used to check for keys)
F4 - Start playback, read from Gex.rec, in the directory where the game executable is located.
F5 - Toggles showing the ingame OSD.
F6 - Forcefully brings you back to the games main menu.
] - To step one frame (this also reloads the input file so you can make changes to the inputs while framestepping.)

Numpad Plus (+) key - Decrease game speed
Numpad Minus (-) key - Increase game speed
Numpad Divide (/) key - Sets game speed to normal.

Note: First frame of input file must be an empty action (so just pad the first line with 1 or however many empty frames you want.)
--------------------------------------------------------
Commands read from input file are in the format of:
   frames, Action
   
(Commands are case insensitive.)
Accepted commands are: <br />
    Left <br />
    Right <br />
    Up <br />
    Down <br />
    Jump <br />
    Tongue <br />
    Sprint <br />
    Tail <br />
    Scene - (can be used to forcefully load a stage, so do Scene, num) <br /> 
    XPos, num - (for teleporting on the x axis, testing purposes) <br />
    YPos, num - (for teleporting on the y axis, testing purposes) <br />
    Runto - to be placed under a line that has commands to be fast forwarded to so you can reset playback and continue stepping if you need to change something.
    
