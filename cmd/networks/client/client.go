package main

import (
	"context"
	"fmt"
	pb "graphicsFinal/generated/proto"
	"log"
	"time"

	"google.golang.org/grpc"
)

func main() {
	conn, err := grpc.Dial("localhost:50051", grpc.WithInsecure())
	if err != nil {
		log.Fatalf("Failed to connect: %v", err)
	}
	defer conn.Close()

	client := pb.NewUserServiceClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	response, err := client.RegisterPlayer(ctx, &pb.RegisterPlayerRequest{})
	if err != nil {
		log.Fatalf("Failed to call MyMethod: %v", err)
	}

	log.Printf("Response from server: %v", response.PlayerId)
	id := response.PlayerId

	for i := range 3 {
		player := &pb.Player{}
		player.PlayerId = id
		player.PosX = int64(i)
		player.PosY = int64(i)
		player.PosZ = int64(i)
		
		gameReq := &pb.GetGameStateRequest{}
		gameReq.Player = player
		response, err := client.GetGameState(ctx, gameReq)
		if err != nil {
			log.Fatalf("Failed to call MyMethod: %v", err)
		}

		me := response.GameState.Players[id]

		fmt.Println("Game state: ", me.PosX, me.PosY, me.PosZ)
	}
}