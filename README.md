# Portal 2 (ASW) in CSGO

This project is aimed at porting P2ASW to the leaked CSGO engine to no longer worry about Alien Swarm's limitations.<br>
Partially used code from https://github.com/EpicSentry/HL2-CSGO<br>

What is P2ASW? Check it out here: https://github.com/EpicSentry/P2ASW/<br>

# Using and building P2GO:
## Windows
- Generate the project using `createportal2projects.bat` and build with VS2019 (2022 with 2019 build tools will work).<br>
- You must install MFC and ATL for V142 (Available under "Individual Components" in the Visual Studio Installer) to build successfully.
- **VS 2022 users:** Do NOT upgrade the solution if prompted to! Hit cancel on the upgrade dialog box.<br>
- Clone the [required game assets repo](https://github.com/CSGOPorts/p2go_game) into `game`.<br>
- Copy over all of your Portal 2 folder to the `game` folder, excluding any `bin` folders.<br>
- Start the game with `rungame.bat`<br>
## Linux
todo

# Debugging the engine
- Set launcher_main as the startup project (if it isn't already) by right clicking it and pressing "Set as Startup Project".
- Right click launcher_main, go to properties and click on the debugging section. Set "Command" to point to your compiled portal2.exe (in the game folder).
- Set "Command Arguments" to "-game "portal2" -insecure -sw" (feel free to add more such as +sv_cheats 1).
- Press "Local Windows Debugger" at the top of Visual Studio to then launch the game and debug it.
