# SnakeBoy
![A screenshot showing SnakeBoy gameplay](screenshot.png?raw=true)

## About
A minimalistic Snake port for Game Boy. Written in C and compiled using [GBDK-2020](https://github.com/gbdk-2020/gbdk-2020).

## Supported Consoles
I test my builds both in [BGB](https://bgb.bircd.org/#downloads) and on an original Game Boy using EZ-Flash Junior. Thanks to [BinaryCounter](https://github.com/binarycounter) for testing on the other devices listed here!
If you find incompatible hardware or other bugs/oddities, please let me know.

- GB/GBC Emulator (such as [BGB](https://bgb.bircd.org/#downloads))
- Game Boy
- Game Boy Advance
- Game Boy Advance SP
- Game Boy Color
- Game Boy Player
- Super Game Boy

## How to play
Get the latest ROM (SnakeBoy.gb) under the [Releases](https://github.com/sebb01/SnakeBoy/releases) page. You can play it an emulator such as [BGB](https://bgb.bircd.org/#downloads), or load it onto a flashcart to play it on a real console.

#### Controls
- D-Pad:&emsp; Control the snake. You cannot directly go in the opposite direction the snake is heading.
- Start:&emsp;&emsp;Start, pause and unpause the game.

## Build from Source
On Windows, you can simply run `make.bat`.
On other platforms supported by GBDK-2020, you can run the following command:
```
\gbdk\bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -Wl-yt1 -Wl-yo4 -Wl-ya0 -msm83:gb -o .\build\SnakeBoy.gb main.c
```

## To do
- Music
- Leaderboard
- Nicer pause screen
- Nicer title screen