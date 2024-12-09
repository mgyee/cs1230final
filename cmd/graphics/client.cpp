#include <iostream>
#include <memory>
#include <string>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <grpcpp/grpcpp.h>
#include "/Users/alexandercueva/Desktop/Brown-University/Junior/1st-Semester/CSCI1680/DEV-ENVIRONMENT/home/cs1230final/cmd/networks/generated/proto/game.grpc.pb.h"
ABSL_FLAG(std::string, target, "localhost:50051", "Server address");
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using userservice::UserService;
using userservice::Player;
using userservice::RegisterPlayerRequest;
using userservice::RegisterPlayerResponse;
using userservice::GetGameStateRequest;
using userservice::GetGameStateResponse;
using userservice::GameState;
class UserServiceClient {
public:
    UserServiceClient(std::shared_ptr<Channel> channel)
        : stub_(UserService::NewStub(channel)) {}
    int64_t RegisterPlayer() {
        // Create request and response objects
        RegisterPlayerRequest request;
        RegisterPlayerResponse response;
        // Create a client context
        ClientContext context;
        // Perform the RPC
        Status status = stub_->RegisterPlayer(&context, request, &response);
        // Check the status and return the player ID
        if (status.ok()) {
            return response.player_id();
        } else {
            std::cerr << "RegisterPlayer RPC failed. "
                      << status.error_code() << ": "
                      << status.error_message() << std::endl;
            return -1;
        }
    }
    GameState GetGameState(const Player& player) {
        // Create request and response objects
        GetGameStateRequest request;
        *request.mutable_player() = player;
        GetGameStateResponse response;
        // Create a client context
        ClientContext context;
        // Perform the RPC
        Status status = stub_->GetGameState(&context, request, &response);
        // Check the status and return the game state
        if (status.ok()) {
            return response.gamestate();
        } else {
            std::cerr << "GetGameState RPC failed. "
                      << status.error_code() << ": "
                      << status.error_message() << std::endl;
            return GameState();
        }
    }
private:
    std::unique_ptr<UserService::Stub> stub_;
};
int main(int argc, char** argv) {
    // Parse command line arguments
    absl::ParseCommandLine(argc, argv);
    // Get the target string from command line flags
    std::string target_str = absl::GetFlag(FLAGS_target);
    // Create a channel and instantiate the client
    UserServiceClient client(
        grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    // Register a player
    int64_t player_id = client.RegisterPlayer();
    if (player_id != -1) {
        std::cout << "Registered player with ID: " << player_id << std::endl;
        // Create a player object
        Player player;
        player.set_player_id(player_id);
        player.set_pos_x(10);
        player.set_pos_y(20);
        player.set_pos_z(30);
        // Get game state
        GameState game_state = client.GetGameState(player);
        // Print out players in the game state
        std::cout << "Current game state has "
                  << game_state.players_size() << " players:" << std::endl;
        for (const auto& existing_player : game_state.players()) {
            std::cout << "Player ID: " << existing_player.player_id()
            << " Position: ("
            << existing_player.pos_x() << ", "
            << existing_player.pos_y() << ", "
            << existing_player.pos_z() << ")" << std::endl;
        }
    }
    return 0;
}
