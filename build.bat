/masm32/bin/ml /c /coff main.asm
cl main.obj http.c
/masm32/bin/link main.obj http.obj
