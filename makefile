all:reciver sender

reciver: ../../../../Downloads/reciver.c
	gcc -o reciver reciver.c

sender: ../../../../Downloads/sender.c
	gcc -o sender sender.c

clean:
	rm -f sender reciver