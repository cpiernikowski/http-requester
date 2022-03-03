/masm32/bin/ml /c /coff main.asm
cl main.obj http.c
/masm32/bin/link /OUT:HTTP-requester.exe main.obj http.obj

del main.exe
del http.obj
del main.obj
