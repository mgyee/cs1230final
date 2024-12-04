package utils

const HELLO_WELCOME_SIZE = 3
const MAX_MESSAGE_SIZE = 1500
const MAX_PACKET_SIZE = 1500
// const KIBIBYTE = 1024
const KIBIBYTE = 16384

const COMMAND_MESSAGE_SIZE = 3


// Golden stack trace debugging
// defer func() {
//         if r := recover(); r != nil {
//             fmt.Println("Recovered from panic:", r)
//             debug.PrintStack() // Print the stack trace
//         }
//     }()