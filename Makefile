build: asm-cforth.c
	gcc asm-cforth.c -o forth -O0

clean:
	rm -f a.out
