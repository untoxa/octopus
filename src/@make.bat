del /F /Q bin\*.*
..\..\gbdk\bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o bin\octopus.o octopus.c
..\..\gbdk\bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -o bin\octopus.gb bin\octopus.o
