SERVER_DEPS = server/server.go
CLIENT_DEPS = client/client.go

all:
	go build -o vserver $(SERVER_DEPS)
	go build -o vclient $(CLIENT_DEPS)
clean: 
	rm -fv vserver
	rm -fv vclient