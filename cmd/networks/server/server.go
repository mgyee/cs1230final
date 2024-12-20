package main

import (
	"context"
	"fmt"
	pb "graphicsFinal/generated/proto"
	"log"
	"net"
	"sync"

	//"structs"

	"google.golang.org/grpc"
)

type server struct {
	pb.UnimplementedUserServiceServer
}

var playerInfo []*pb.Player
var gameInfoMutex *sync.Mutex
var id int32 = 0
var idMutex *sync.Mutex

func updatePlayerInformation(id int32, info pb.Player) {
	gameInfoMutex.Lock()
	defer gameInfoMutex.Unlock()

	playerInfo[id] = &info
}

func printPlayerInfo() {
	gameInfoMutex.Lock()
	defer gameInfoMutex.Unlock()

	for _, player := range playerInfo {
		fmt.Printf("Player %d: with position, %f, %f, %f\n", player.PlayerId, player.PosX, player.PosY, player.PosZ)
	} 
}

// Implement each RPC method
func (s *server) RegisterPlayer(ctx context.Context, req *pb.RegisterPlayerRequest) (*pb.RegisterPlayerResponse, error) {
	idMutex.Lock()
	p_id := id
	id += 1
	idMutex.Unlock()

	gameInfoMutex.Lock()
	playerInfo = append(playerInfo, &pb.Player{})
	gameInfoMutex.Unlock()

	printPlayerInfo()
	
	return &pb.RegisterPlayerResponse{
		PlayerId: p_id,
	}, nil
}
func (s *server) GetGameState(ctx context.Context, req *pb.GetGameStateRequest) (*pb.GetGameStateResponse, error) {
	updatePlayerInformation(req.Player.PlayerId, *req.Player)

	gameInfoMutex.Lock()
	curGameState := pb.GameState{
		Players: playerInfo,
	}
	gameInfoMutex.Unlock()

	printPlayerInfo()

	return &pb.GetGameStateResponse{
		GameState: &curGameState,
	}, nil
}

func main() {
	// change the num?
	playerInfo = make([]*pb.Player, 0)
	gameInfoMutex = &sync.Mutex{}
	idMutex = &sync.Mutex{}
	// Create a listener on TCP port
	lis, err := net.Listen("tcp", ":50051")
	if err != nil {
		log.Fatalf("Failed to listen: %v", err)
	}

	// Create a gRPC server object
	grpcServer := grpc.NewServer()

	// Attach the UserService implementation
	pb.RegisterUserServiceServer(grpcServer, &server{})

	// Start the server
	log.Println("Starting gRPC server on :50051")
	if err := grpcServer.Serve(lis); err != nil {
		log.Fatalf("Failed to serve: %v", err)
	}
}