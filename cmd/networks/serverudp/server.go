package main

import (
	//"context"
	"fmt"

	//pb "graphicsFinal/generated/proto"
	"bytes"
	"encoding/binary"
	"log"
	"math"
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

// keeps a set of the players that need to experience knockback
var globalSet = make(map[int32]struct{})
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

func sendGameState(conn *net.UDPConn, addr *net.UDPAddr, myPlayerId int32) {
	
	gameInfoMutex.Lock()
	currGameState := &GameState{GamePlayerInfo: playerInfo}
	gameInfoMutex.Unlock()
	buffer, err := marshalGameState(currGameState, myPlayerId)
	if err != nil {
		fmt.Println("Error marshalling game state:", err)
	}

	_, err = conn.WriteToUDP(buffer, addr)

	if err != nil {
		fmt.Println("Error writing to UDP:", err)
		return
	}
}

func isCloseTo(value, target, epsilon float64) bool {
	return math.Abs(value-target) < epsilon
}

func findClosestAndStoreHitEvent(myself Player) {
	threshold := 5.0
	closestPlayer := int32(-1)
	closestDistance := 10000000.0
	for _, player := range playerInfo {
		if player.PlayerId == myself.PlayerId {
			continue
		}

		distance := math.Sqrt(math.Pow(float64(player.PosX - myself.PosX), 2) + math.Pow(float64(player.PosY - myself.PosY), 2) + math.Pow(float64(player.PosZ - myself.PosZ), 2))
		// make sure the distance you calculated is less than the threshold, and the distance is less than the closest distance
		if distance < threshold && distance < closestDistance {
			closestDistance = distance
			closestPlayer = player.PlayerId
		}
	}

	if closestPlayer == -1 {
		fmt.Println("No valid player for knockback")
		return
	}

	// we have now found the "closest" player within the threshold, put them
	// in this set and on the next query, they'll be knocked back
	// clear it once knockback gets applied once
	globalSet[closestPlayer] = struct{}{}
}

func main() {
	epsilon := 1e-9  
	// change the num?
	playerInfo = make([]*Player, 0)
	gameInfoMutex = &sync.Mutex{}
	idMutex = &sync.Mutex{}
	// Create a listener on TCP port
	serverAddr := "0.0.0.0:12345"

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
	// idea is that we read and see who sent it to us
	// use a map to store address to player id

	// player 1 sends a hit event
	// we see the hit event, and then identify the player that is closest to player 1 (lets say its player 2), put this in a map
	// afterwards, we can store this in a map, when player 2 queries (remember we can see its id from the position update it sends)
	// we can send the hit event to player 2, and player 2 will take care of moving themselves back
	// we can do this by setting the velocity to be -1? or something like that
	// player 
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
			// what if there are no results?
			// velocity x being close to 1000 signifies a hit event
			if (isCloseTo(float64(player.VelX), 1000, epsilon)) {	
				fmt.Println("Hit event received")
				findClosestAndStoreHitEvent(player)
			}

			updatePlayerInformation(player.PlayerId, &player)
			sendGameState(conn, addr, player.PlayerId)
		}

		// printPlayerInfo()
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

func marshalGameState(gameState *GameState, myPlayerId int32) ([]byte, error) {
	buf := new(bytes.Buffer)

	// Write each value to the buffer
	for _, player := range gameState.GamePlayerInfo {
		if err := marshalPlayer(player, buf, myPlayerId); err != nil {
			return nil, fmt.Errorf("error encoding player %d: %w", player.PlayerId, err)
		}
	}

	return buf.Bytes(), nil
}

func marshalPlayer(player *Player, buf *bytes.Buffer, myPlayerId int32) (error) {

	var values []interface{}
	// For knockback, we have to communicate the event to the player that was knocked back
	// Thus we pass in myPlayerId (the current person that we received a udp packet from)
	// When marshalling the player, if the id matches, and we exist within the global set,
	// then we change our velocity to -1000.0 which we will then read in the client
	// Don't change anything in player info struct because this should just be a temporary change
	// to signify a hit event
	_, ok := globalSet[player.PlayerId];
	if player.PlayerId == myPlayerId && ok {
		delete(globalSet, player.PlayerId)
		values = []interface{}{
			int32(player.PlayerId),
			float32(player.PosX),
			float32(player.PosY),
			float32(player.PosZ),
			float32(-1000.0),
			float32(player.VelY),
			float32(player.VelZ),
		}
	} else {
		values = []interface{}{
			int32(player.PlayerId),
			float32(player.PosX),
			float32(player.PosY),
			float32(player.PosZ),
			float32(player.VelX),
			float32(player.VelY),
			float32(player.VelZ),
		}
	}

	// Write each value to the buffer
	for _, value := range values {
		if err := binary.Write(buf, binary.BigEndian, value); err != nil {
			return fmt.Errorf("error encoding value %v: %w", value, err)
		}
	}

	return nil
}