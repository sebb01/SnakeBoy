set LCC=".\gbdk\bin\lcc"
set FLAGS=-Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -Wl-yt1 -Wl-yo4 -Wl-ya0 -msm83:gb

%LCC% %FLAGS% -o .\build\SnakeBoy.gb main.c

REM Clean up build files
cd .\build
del *.o *.lst *.ihx *.map *.noi *.asm *.sym