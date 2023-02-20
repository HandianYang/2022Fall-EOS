all: server.c client.c
	gcc server.c -o server.o sockop.c
	gcc client.c -o client.o sockop.c
clean:
	rm -rf server.o client.o