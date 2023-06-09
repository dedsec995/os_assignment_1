monks: monks.c
	gcc monks.c -o monks -pthread

diner: diner.c
	gcc diner.c -o diner -pthread

family: family.c
	gcc family.c -o family -pthread

shell: shell.c
	gcc shell.c -o shell

run:
	gcc shell.c -o shell && clear && ./shell

clean:
	rm -rf shell && clear

demo:
	gcc demo.c -o demo -pthread && clear && ./demo

ddemo:
	rm -rf demo && clear
