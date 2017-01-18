ALL:y86emul y86dis
y86emul:
	gcc -g -Wall y86emul.c -lm -o y86emul.out
y86dis:
	gcc -g -Wall y86dis.c -lm -o y86dis.out
clean:
	rm -rf *.out