
overlord: main.c
	$(CC) main.c -o overlord

PHONY += clean
clean:
	rm -f overlord
