GCC = gcc -Wall
PACKAGES = src/queue.c src/utils.c
PERMISSION = chmod +x

kill:
	fuser 8001/udp -k -i || true
	fuser 8002/udp -k -i || true
	fuser 8003/udp -k -i || true
	fuser 8004/udp -k -i || true

compile:
	$(GCC) src/server.c $(PACKAGES) -o bin/server
	$(GCC) src/client.c $(PACKAGES) -o bin/client

run-server:
	$(PERMISSION) bin/server
	bin/server $(IP_SERVER) $(PORT_SERVER)

run-client:
	$(PERMISSION) bin/client
	bin/client $(IP_SERVER) $(PORT_SERVER_2) $(IP_CLIENT) $(PORT_CLIENT)
