set LCC=".\.\gbdk\bin\lcc"

%LCC% -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -Wl-yt1 -Wl-yo4 -Wl-ya0 -msm83:gb -o .\build\GBSnake.gb main.c
cd .\build
del *.o *.lst *.ihx *.map *.noi *.asm *.sym