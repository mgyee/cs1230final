SERVER_DEPS = server/server.go
SERVERUDP_DEPS = serverudp/server.go
CLIENT_DEPS = client/client.go

all:
	go build -o vserver $(SERVER_DEPS)
	go build -o vserver_udp $(SERVERUDP_DEPS)
	go build -o vclient $(CLIENT_DEPS)
clean: 
	rm -fv vserver
	rm -fv vclient
	rm -fv vserver_udp