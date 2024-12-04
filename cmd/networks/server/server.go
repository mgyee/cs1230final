package main

import (
	"encoding/binary"
	"fmt"
	"graphicsFinal/structs"
	"graphicsFinal/utils"
	"io"
	"log"
	"net"
	"os"
	"runtime/debug"
	"strconv"
	"sync"
	"time"
)

const PLAYER_MESSAGE_SIZE = 24

var clientsList [](*structs.Client)
var clientListMutex sync.Mutex
var udpPortConnectionMap map[uint16](*net.UDPConn) = make(map[uint16](*net.UDPConn))
var udpPortConnectionMapMutex sync.Mutex

var stations [](structs.Station)
var stationNumberMutex sync.Mutex
var stationNumber int = 0
var IPADDR string = "127.0.0.1"
var setup bool = false
var acceptingClients bool = true

//  connectionN has the form <IPAddr>:<UDPPort>.

func clientRunner(conn net.Conn, num_stations uint16) {
	defer func() {
        if r := recover(); r != nil {
            fmt.Println("Recovered from panic:", r)
            debug.PrintStack()
        }
    }()

	id := 0

	// Add to client list
	clientListMutex.Lock()
	if !acceptingClients {
		clientListMutex.Unlock()
		return
	}
	new_client := structs.Client{Id: id, ControlConn: conn}
	clientsList = append(clientsList, &new_client)
	clientListMutex.Unlock()

	defer cleanupClient(&new_client)
	// make a buffer of size utils.MAX_MESSAGE_SIZE bytes
	data := make([]byte, utils.MAX_MESSAGE_SIZE)

	// Close the connection on exit. If the connection is already closed,
	// conn.Close() can be called safely multiple times
	defer conn.Close()
	totalBytesRead := 0
	// position, x, y , z;
	messageSize := PLAYER_MESSAGE_SIZE
	for totalBytesRead < messageSize {
        read, err := conn.Read(data[totalBytesRead:messageSize])

		if err != nil {
			fmt.Println("connection closed")
			return
		}

        totalBytesRead += read
    }

    if totalBytesRead != 12 {
		return
    }
}

// Cleans up the client from the station list, a list of clients is still
// kept in case the server is exited, so we can iterate and kill
// all connections
func cleanupClient(client *structs.Client) {
	// fmt.Println("Cleaning up client")
	if client.StationNumber != -1 {
		fromStation := &stations[client.StationNumber]
		fromStation.StationMutex.Lock()
		removeClient(&fromStation.ClientList, client)
		fromStation.StationMutex.Unlock()
		removeUdpPortMap(client.UdpPort, client.StationNumber)
		client.ControlConn.Close()
	}
}

// MUST be called with station mutex LOCKED
func removeUdpPortMap(udpPort uint16, stationNumber int) {
	for i, _ := range stations {
		station := &stations[i]
		station.StationMutex.Lock()
		_, ok := station.UdpPorts[udpPort]
		if ok {
			delete(station.UdpPorts, udpPort)
			for i, client := range station.ClientList {
				if client.UdpPort == udpPort {
					clientList := &station.ClientList
					*clientList = append((*clientList)[:i], (*clientList)[i+1:]...)
					break
				}
			}
		}
		station.StationMutex.Unlock()
	}
}

func makeInvalidCommand(message string) structs.InvalidCommand {
	invalidCommand := structs.InvalidCommand{ReplyType: uint8(4), ReplyStringSize: uint8(len(message)), ReplyString: []byte(message)}
	return  invalidCommand
}

func listen_handler(listener *net.TCPListener, num_stations int) {
	defer listener.Close()
	for {
		new_conn, err := listener.Accept()
		if err != nil {
			return
		}
		go clientRunner(new_conn, uint16(num_stations))
	}
}

func stationRunner(filename string, wg *sync.WaitGroup, done chan bool) {
	file, err := os.Open(filename)
	if err != nil {
		fmt.Println("Error opening file: ", filename)
		wg.Done()
		return
	}
	defer file.Close()
	stationNumberMutex.Lock()
	// Lock the station data struct to safely insert in the correct position
	myStationNumber := stationNumber
	oneStation := structs.Station{StationMutex: &sync.Mutex{}, StationNumber: myStationNumber, Filename: filename, ClientList: make([](*structs.Client), 0), UdpPorts: make(map[uint16]*structs.ConFreq)}
	stations = append(stations, oneStation)
	stationNumber++
	stationNumberMutex.Unlock()

	var announce_msg []byte
	announce_msg = append(announce_msg, 0x03)
	announce_msg = append(announce_msg, byte(len(filename)))
	announce_msg = append(announce_msg, []byte(filename)...)
	done <- true
	wg.Done()
	for !setup {
		continue
	}

	defer stationCleanup(myStationNumber)

	go sendAnnounce(announce_msg, myStationNumber)
	buffer := make([]byte, 1024)
	sendNewAnnounce := false
	for {
		if !acceptingClients{
			return
		}
		// Offset maintiained by the file)
		read, err := file.Read(buffer)
		if err == io.EOF {
			file.Seek(0, 0)
			sendNewAnnounce = true
		} else if err != nil {
			log.Fatal(err, "Error reading from file: ", filename)
			log.Fatal("Error reading from file: ", filename)
		}

		sendAudioData(buffer[:read], myStationNumber)

		if sendNewAnnounce {
			go sendAnnounce(announce_msg, myStationNumber)
			sendNewAnnounce = false
		}

		time.Sleep(62500 * time.Microsecond)
	}
}

// Cleans up the station by closing all udp connections
func stationCleanup(stationNumber int) {
	myStation := &stations[stationNumber]
	myStation.StationMutex.Lock()
	for _, connFreq := range myStation.UdpPorts {
		connFreq.Conn.Close()
	}
	myStation.StationMutex.Unlock()
}

// Sends audio data to all udp connections that are on this station
func sendAudioData(audio_data []byte, stationNumber int) {
	stations[stationNumber].StationMutex.Lock()
	defer stations[stationNumber].StationMutex.Unlock()

	
	for _, udpConn := range stations[stationNumber].UdpPorts {
		(*udpConn.Conn).Write(audio_data)
	}
}

// Sends the announce message to all clients of a certain station
func sendAnnounce(announce_msg []byte, stationNumber int) {
	stations[stationNumber].StationMutex.Lock()
	defer stations[stationNumber].StationMutex.Unlock()
	for _, client := range stations[stationNumber].ClientList {
		client.ControlConn.Write(announce_msg)
	}
}

// MUST ONLY BE CALLED WHEN stationStructMutex IS LOCKED
func removeClient(clientList *[](*structs.Client), removeClient *structs.Client) {
	for i, client := range *clientList {
		if *client == *removeClient {
			*clientList = append((*clientList)[:i], (*clientList)[i+1:]...)
			return
		}
	}
}

func main() {
    args := os.Args
	if len(args) < 3 {
		log.Fatal("Usage: ./snowcast_server <listen port> <file0> [file 1] [file 2] ...")
	}

	var wg sync.WaitGroup
	done := make(chan bool)
	for _, filename := range args[2:] {
		wg.Add(1)
		go stationRunner(filename, &wg, done)
		<-done
		done = make(chan bool)
	}

	addr, err := net.ResolveTCPAddr("tcp4", IPADDR + ":" + args[1])
	if err != nil {
		// Does both of these steps for us
		log.Fatal(err)
	}

	// := is a new assignment operator; whereas = is the assignment operator
	// (for reassignment)
	listener, err := net.ListenTCP("tcp4", addr)
	if err != nil {
		log.Fatal(err)
	}

	wg.Wait()
	
	setup = true
	defer listener.Close()
	stationNumberMutex.Lock()
	// fmt.Println(len(args[2:]))
	// fmt.Println(stationNumber)
	go listen_handler(listener, stationNumber)
	stationNumberMutex.Unlock()
}