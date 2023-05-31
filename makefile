monks: monks.c
	gcc monks.c -o monks -pthread

diner: diner.c
	gcc diner.c -o diner -pthread

family: family.c
	gcc family.c -o family -pthread

my:
	gcc shell.c -o shell -pthread && clear && ./shell

clean:
	rm -rf shell && clear