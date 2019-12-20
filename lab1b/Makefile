#NAME: Elwyn Cruz
#EMAIL: ElwynCruz@g.ucla.edu
#ID: 104977892
.SILENT:

default:
	gcc -lmcrypt -Wall -Wextra -o lab1b-client lab1b-client.c
	gcc -lmcrypt -Wall -Wextra -o lab1b-server lab1b-server.c
lab1b-server:
	gcc -lmcrypt -Wall -Wextra -o lab1b-server lab1b-server.c
lab1b-client:
	gcc -lmcrypt -Wall -Wextra -o lab1b-client lab1b-client.c
clean:
	rm -f lab1b-client lab1b-server *.tar.gz *.txt
dist:
	tar -czf lab1b-104977892.tar.gz README lab1b-client.c lab1b-server.c Makefile my.key
