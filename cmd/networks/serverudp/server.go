package main

import (
	//"context"
	"fmt"

	//pb "graphicsFinal/generated/proto"
	"bytes"
	"encoding/binary"
	"log"
	"net"
	"sync"
	//"structs"
	//"google.golang.org/grpc"
)

// type server struct {
// 	pb.UnimplementedUserServiceServer
// }

type Player struct {
	PlayerId             int32    
	PosX                 float32  
	PosY                 float32  
	PosZ                 float32  
	VelX                 float32  
	VelY                 float32  
	VelZ                 float32
}

type GameState struct {
	GamePlayerInfo []*Player
}



var playerInfo []*Player
var gameInfoMutex *sync.Mutex
var id int32 = 0
var idMutex *sync.Mutex

func updatePlayerInformation(id int32, info *Player) {
	gameInfoMutex.Lock()
	defer gameInfoMutex.Unlock()

	playerInfo[id] = info
}

func printPlayerInfo() {
	gameInfoMutex.Lock()
	defer gameInfoMutex.Unlock()

	for _, player := range playerInfo {
		fmt.Printf("Player %d: with position, %f, %f, %f\n", player.PlayerId, player.PosX, player.PosY, player.PosZ)
	} 
}

// Implement each RPC method
func RegisterPlayer() int32 {
	idMutex.Lock()
	p_id := id
	id += 1
	idMutex.Unlock()

	gameInfoMutex.Lock()
	playerInfo = append(playerInfo, &Player{PlayerId:  p_id})
	fmt.Println("Player registered with id:", p_id)
	gameInfoMutex.Unlock()
	
	return p_id
}

func sendPlayerId(conn *net.UDPConn, addr *net.UDPAddr, player *Player) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.BigEndian, player.PlayerId)
	if err != nil {
		fmt.Println("Error encoding integer:", err)
		return
	}
	_, err = conn.WriteToUDP(buf.Bytes(), addr)

	if err != nil {
		fmt.Println("Error writing to UDP:", err)
		return
	}
}

func sendGameState(conn *net.UDPConn, addr *net.UDPAddr) {
	
	gameInfoMutex.Lock()
	currGameState := &GameState{GamePlayerInfo: playerInfo}
	gameInfoMutex.Unlock()
	buffer, err := marshalGameState(currGameState)
	if err != nil {
		fmt.Println("Error marshalling game state:", err)
	}

	_, err = conn.WriteToUDP(buffer, addr)

	if err != nil {
		fmt.Println("Error writing to UDP:", err)
		return
	}
}

func main() {
	// change the num?
	playerInfo = make([]*Player, 0)
	gameInfoMutex = &sync.Mutex{}
	idMutex = &sync.Mutex{}
	// Create a listener on TCP port
	serverAddr := "127.0.0.1:12345"

	// Resolve the UDP address
	udpAddr, err := net.ResolveUDPAddr("udp", serverAddr)
	if err != nil {
		log.Fatalf("Failed to resolve address: %v", err)
	}

	// create a udp listener
	conn, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		log.Fatalf("Failed to listen on UDP: %v", err)
	}
	defer conn.Close()

	buffer := make([]byte, 1024)
	for {
		n, addr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("Error reading data: %v", err)
			continue
		}

		var player Player
		bytesToPlayerStruct(buffer, n, &player)
		if player.PlayerId == -1 {
			fmt.Println("Registering player")
			p_id := RegisterPlayer()
			player.PlayerId = p_id
			sendPlayerId(conn, addr, &player)
		} else {
			updatePlayerInformation(player.PlayerId, &player)
			sendGameState(conn, addr)
		}

		printPlayerInfo()
	}
}


func bytesToPlayerStruct(buf []byte, n int, player *Player) {
	data := buf[:n]

	reader := bytes.NewReader(data[:4])
	err := binary.Read(reader, binary.BigEndian, &player.PlayerId)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	if (n == 4) {
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

func marshalGameState(gameState *GameState) ([]byte, error) {
	buf := new(bytes.Buffer)

	// Write each value to the buffer
	for _, player := range gameState.GamePlayerInfo {
		if err := marshalPlayer(player, buf); err != nil {
			return nil, fmt.Errorf("error encoding player %d: %w", player.PlayerId, err)
		}
	}

	return buf.Bytes(), nil
}

func marshalPlayer(player *Player, buf *bytes.Buffer) (error) {

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