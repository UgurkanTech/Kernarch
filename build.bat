nasm -f win64 test.asm -o test.obj

cl /EHsc /c main.cpp

link main.obj test.obj /out:main.exe