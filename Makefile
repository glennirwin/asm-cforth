build: asm-cforth.c
	gcc -o forth asm-cforth.c

clean:
	rm -f a.out
