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
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
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

	buffer := make([]byte, 256)
	// Read in the first thing, don't care what it is, just make
	// a request for a player id
	num, err := cppConn.Read(buffer)

	if err != nil {
		log.Fatalf("Failed to read from cppSock: %v", err)
	}
	// _ = cppConn
	_ = num


	// probably do more robust later
	
	// if string(buffer) != "c" {
	// 	log.Fatalf("Wrong connection from cpp client")
	// }

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

	for {
		// double check how this works in go, not reseting the buffer
		buffer := make([]byte, 256)
		num, err = cppConn.Read(buffer)

		if err != nil {
			log.Fatalf("Failed to read from socket: %v", err)
		}
		// unmarshal the stuff here
		// send player update to the server
		// with the received response, send it over the tcpconn
		gameReq := &pb.GetGameStateRequest{}
		// fill this out
		var player pb.Player
		bytesToPlayerStruct(buffer, num, &player)
		gameReq.Player = &player
		ctx, cancel := context.WithTimeout(context.Background(), time.Second)
		response, err := client.GetGameState(ctx, gameReq)
		cancel()
		if status.Code(err)  == codes.DeadlineExceeded {
			// send a random byte to say error
			fmt.Println("skipped the deadline")
			// _, err = cppConn.Write(prevState)
			// if err != nil {
			// 	log.Fatalf("Failed to write to the socket %v", err)
			// }
			continue
		} else if err != nil {
			log.Fatalf("Failed to call MyMethod: %v", err)
		}
		gameStateBuffer, err := marshalGameState(response.GameState)
		//prevState = gameStateBuffer
		if err != nil {
			log.Fatalf("Failed to marshall game state: %v", err)
		}
		// write the size of the response back to our cpp client
		sent, err := cppConn.Write(gameStateBuffer)
		if err != nil {
			log.Fatalf("Failed to write to the socket %v", err)
		}
		_ = sent
		//fmt.Printf("sent %d bytes\n", sent)
	}
}

func bytesToPlayerStruct(buf []byte, n int, player *pb.Player) {
	data := buf[:n]

	reader := bytes.NewReader(data[:4])
	err := binary.Read(reader, binary.BigEndian, &player.PlayerId)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	offset := 4
	gameData := make([]float32, 6)

	for i := range 6 {
		reader := bytes.NewReader(data[offset:offset + 4])
		err := binary.Read(reader, binary.BigEndian, &gameData[i])
		if err != nil {
			fmt.Println("Error:", err)
			return
		}
		offset += 4
	}

	player.PosX = gameData[0]
	player.PosY = gameData[1]
	player.PosZ = gameData[2]
	player.VelX = gameData[3]
	player.VelY = gameData[4]
	player.VelZ = gameData[5]	
}

func marshalGameState(gameState *pb.GameState) ([]byte, error) {
	buf := new(bytes.Buffer)

	// Write each value to the buffer
	for _, player := range gameState.Players {
		if err := marshalPlayer(player, buf); err != nil {
			return nil, fmt.Errorf("error encoding player %d: %w", player.PlayerId, err)
		}
	}

	return buf.Bytes(), nil
}

func marshalPlayer(player *pb.Player, buf *bytes.Buffer) (error) {

	// List of values to encode
	values := []interface{}{
		int32(player.PlayerId),
		float32(player.PosX),
		float32(player.PosY),
		float32(player.PosZ),
		float32(player.VelX),
		float32(player.VelY),
		float32(player.VelZ),
	}

	// Write each value to the buffer
	for _, value := range values {
		if err := binary.Write(buf, binary.BigEndian, value); err != nil {
			return fmt.Errorf("error encoding value %v: %w", value, err)
		}
	}

	return nil
}