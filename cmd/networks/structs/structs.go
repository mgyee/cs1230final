package structs

import (
	"net"
	"sync"
)

// Exported members MUST start with an uppercase
// otherwise go treats them as unexported and only accessible within the package

// CLIENT MESSAGES
// CommandType must be 0 for Hello Message
type Hello struct {
	CommandType uint8 `json:"command_type"`
	UdpPort uint16 `json:"udpPort"`;
}

// CommandType must be 1 for SetStation Message
type SetStation struct {
	CommandType uint8 `json:"commandType"`;
	StationNumber uint16 `json:"stationNumber"`;
}

// SERVER_MESSAGES
// CommandType must be 2 for Welcome Message
type Welcome struct {
	ReplyType uint8 `json:"replyType"`;
    NumStations uint16 `json:"numStations"`;
}

// CommandType must be 3 for Announce Message
type Announce struct {
	ReplyType uint8 `json:"replyType"`;
	SongnameSize uint8 `json:"songnameSize"`;
	Songname []byte `json:"songname"`;
}

// CommandType must be 4 for InvalidCommand Message
type InvalidCommand struct {
	ReplyType uint8 `json:"replyType"`;
	ReplyStringSize uint8 `json:"replyStringSize"`;
	ReplyString []byte `json:"replyString"`;
}

// DATA STRUCTURES
type Station struct {
	StationMutex *sync.Mutex
	StationNumber int
	Filename string
	ClientList [](*Client)
	UdpPorts map[uint16](*ConFreq)
}

type ConFreq struct {
	Conn *net.UDPConn
	Freq int
}

type Client struct {
	ControlConn net.Conn
	Id int
}