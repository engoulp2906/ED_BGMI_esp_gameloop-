!. Strictly Educational or Research purpose only. !

BGMI 32-bit Gameloop Basic External ESP — README


Overview

This project is a basic external ESP (Extra Sensory Perception) cheat for BGMI (Battlegrounds Mobile India) running inside the 32-bit Gameloop emulator on Windows. The ESP is designed to run externally (outside the game process) and provides a minimalistic visual aid by drawing a simple dot on enemy players on the screen.



Features


External ESP: No injection or modification of the game process; reads game memory externally.

Dot ESP: Draws a small colored dot on the screen at the enemy’s position.

Enemy detection: Reads enemy player positions and status (alive/dead) using game memory offsets.

World to Screen: Converts 3D world coordinates of enemies to 2D screen coordinates using the game's view matrix.

Dynamic UE4 base address finding: Automatically locates the UE4 module base address inside Gameloop to handle address randomization.

Overlay: Creates a transparent, click-through overlay window on top of Gameloop to render ESP dots without interfering with gameplay.



How It Works — Technical Explanation



Process Attachment:

The ESP attaches to the Gameloop process (dnplayer.exe) with read permissions using Windows API.




Dynamic Base Address Resolution:

Since the game module base address changes every launch, the ESP enumerates loaded modules in Gameloop to find the base address of the UE4 game module dynamically.




Memory Reading:

Using the known UE4 base address and provided offsets (specific to BGMI's UE4 build), the ESP reads:



The global GWorld pointer.

The PersistentLevel and actors array.

Each actor’s root component and relative location (position).

Actor status flags such as team ID and alive/dead state.




World to Screen Projection:

The ESP reads the game’s view matrix from memory and applies matrix transformations to convert enemy 3D world positions into 2D screen coordinates.




Rendering Overlay:

A transparent overlay window is created above the Gameloop window. The ESP draws small dots at the screen coordinates of all alive enemy players.





Important Offsets Used

| Offset Name          | Description                        |
|----------------------|----------------------------------|
| GWorld             | Base pointer to the global world |
| GWorld + 0x3C      | Actual GWorld pointer           |
| PersistentLevel     | Pointer to the level object       |
| Actors Array        | Pointer to array of actors        |
| Actors Count        | Number of actors in the array     |
| RootComponent       | Actor’s root component pointer    |
| RelativeLocation    | Actor’s position vector           |
| TeamId              | Player team identifier            |
| IsDead              | Dead/alive status flag            |
| ViewMatrixBase      | Base address of the view matrix   |



Usage Instructions



Launch Gameloop and start BGMI.

Enter an active match to ensure game memory is fully loaded.




Run the ESP executable as Administrator.

This is required to read memory from the emulator process.




The ESP will:



Attach to Gameloop process.

Dynamically find the UE4 base address.

Read enemy player data and draw dots on enemies on a transparent overlay.




You should see small dots on the screen representing enemy positions.





Limitations and Notes


This ESP only draws dots on enemy players; no boxes, skeletons, or other visuals.

The ESP currently does not filter teammates; all alive actors are shown.

The overlay uses basic GDI drawing and may flicker; improvements like double buffering or DirectX can enhance performance.

Offsets are specific to the current BGMI version and may change with updates; maintenance is required.

Use this tool only for educational and authorized testing purposes.

Running cheats may violate game terms and result in bans.



Development and Contributions


The project is a starting point for learning external memory reading and overlay rendering.

Contributions to add features like team filtering, smooth rendering, and offset updating are welcome.

Please ensure ethical use and do not distribute cheats for malicious purposes.



Disclaimer

This project is intended for educational purposes only. The author is not responsible for any misuse or consequences arising from its use.



Contact

Discord : https://discord.gg/AcUyaxZZbg



This README provides a clear understanding of the ESP’s purpose, design, and usage for anyone browsing your GitHub repository.
