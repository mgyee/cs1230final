package main

import (
	"bytes"
	"context"
	"encoding/binary"
	"fmt"
	pb "graphicsFinal/generated/proto"
	"log"
	"net"
	"time"

	"google.golang.org/grpc"
)

func main() {

	lis, err := net.Listen("tcp", ":50050")
	if err != nil {
		log.Fatalf("Failed to listen: %v", err)
	}

	cppConn, err := lis.Accept()

	if err != nil {
		log.Fatalf("Failed to connect to cpp client: %v", err)
	}

	defer cppConn.Close()
	fmt.Println("Past connection")

	buffer := make([]byte, 1600)
	// Read in the first thing, don't care what it is, just make
	// a request for a player id
	num, err := cppConn.Read(buffer)

	if err != nil {
		log.Fatalf("Failed to read from cppSock: %v", err)
	}
	// _ = cppConn
	_ = num


	// probably do more robust later
	if string(buffer) != "connecting" {
		log.Fatalf("Wrong connection from cpp client")
	}

	conn, err := grpc.Dial("localhost:50051", grpc.WithInsecure())
	if err != nil {
		log.Fatalf("Failed to connect: %v", err)
	}
	defer conn.Close()

	client := pb.NewUserServiceClient(conn)

	// response, err := client.RegisterPlayer(ctx, &pb.RegisterPlayerRequest{})
	response, err := client.RegisterPlayer(context.Background(), &pb.RegisterPlayerRequest{})
	if err != nil {
		log.Fatalf("Failed to call MyMethod: %v", err)
	}

	log.Printf("Response from server: %v", response.PlayerId)
	// Should write back the player id to the cpp client
	buf := new(bytes.Buffer)
	err = binary.Write(buf, binary.BigEndian, int32(response.PlayerId))
	if err != nil {
		fmt.Println("Error encoding integer:", err)
		return
	}
	_, err = cppConn.Write(buf.Bytes())
	if (err != nil) {
		log.Fatalf("Failed to write to cppSock: %v", err)
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	//id := response.PlayerId
	

	for {
		// double check how this works in go, not reseting the buffer
		num, err = cppConn.Read(buffer)
		// unmarshal the stuff here
		// send player update to the server
		// with the received response, send it over the tcpconn
		gameReq := &pb.GetGameStateRequest{}
		// fill this out
		var player pb.Player
		gameReq.Player = player
		response, err := client.GetGameState(ctx, gameReq)
		if err == context.DeadlineExceeded {
			// send a random byte to say error
			continue
		} else if err != nil {
			log.Fatalf("Failed to call MyMethod: %v", err)
		}

		gameStateBuffer, err := marshalGameState(respoinse.GameState)
		// write the size of the response back to our cpp client
		_, err = cppConn.Write(gameStateBuffer)
	}
}

func bytesToPlayerStruct() {
	
}

func marshalGameState(gameState *pb.GameState) ([]byte, error) {
	buf := new(bytes.Buffer)

	// Write each value to the buffer
	for _, player := range gameState.Players {
		if err := marshalPlayer(&player, buf) {
			return nil, fmt.Errorf("error encoding player %p: %w", player, err)
		}
	}

	return buf.Bytes(), nil
}

func marshalPlayer(player *pb.Player, buf *bytes.Buffer) (error) {

	// List of values to encode
	values := []interface{}{
		int32(player.PlayerId),
		float64(player.PosX),
		float64(player.PosY),
		float64(player.PosZ),
		float64(player.VelX),
		float64(player.VelY),
		float64(player.VelZ),
	}

	// Write each value to the buffer
	for _, value := range values {
		if err := binary.Write(buf, binary.BigEndian, value); err != nil {
			return fmt.Errorf("error encoding value %v: %w", value, err)
		}
	}

	return nil

}