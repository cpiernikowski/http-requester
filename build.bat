/masm32/bin/ml /c /coff main.asm
cl main.obj networking.c
/masm32/bin/link /SUBSYSTEM:WINDOWS main.obj networking.obj