# Temu Bed Wars 
## Project description

We created a multiplayer game as a final project for both cs1230 and cs1680. Up to 4 players can join an arena, where each player controls one of the staff members for cs1230: Daniel, Gavin, Evan, or Faisal. 
Each player can control their fog density setting as well FXAA filter. The arena is taken from project 6, realtime, with an additional skybox that enhances the visual component. Players are able to move with WASD keys, jump with space, and control the camera with their mouse. If a player hits the E key next to another player, the other player will get knocked back, allowing for some miniature player versus player fighting.
If a player falls of the arena, they will get respawned in the center, allowing for infinite fun! 

## Project Structure

### Graphics

The "cmd/graphics" directory contains the graphics part of the project. Most of our implementation was handled in the realtime.cpp/.h files, extending the functionality of project 6. 
To run the project, a runtime argument needs to provides, which is the IP of the host server. To run just the graphics part, any IP can be provided, since the update packets will just be lost with no effect on the player. 
To run the multiplayer setting, a go server, located at cmd/networks/vserver_udp must be first run, and the IP of that server must be provided as a runtime argument. 

We worked on the following technical features for graphics: 
- Skybox: Found in the createSkybox() function as well as the additional shader in resources/shaders.
- The fog effect: bound in every paintGL call, most of its comprehensive implementation is done within the default shader. The fog can be controlled with the UI slider that adjusts the density of the visual effect.
- FXAA filter: enabled by a settings boolean, the core of its work is done on the postprocessing shader. When enabled, it reduces the "jagged" pixel visual artifacts which smoothes out the picture. 
- Collision detection: every object has a bounding box, and every timerEvent loop checks the coordinates of other objects to determine collisions. This way, players will stop if they collide and can register hit and knockback events. 
- Texture mapping: every cube that is connected to a player renders a texture on top of their cube, using texture mapping and UV coordinates generated from the trimesh code. 
- Entity Component System - every "object" in our scene is represented as an entity, where entities can have a "Renderable" component that can be drawn on the screen, and a "Player" component that updates the position and receives updates for the multiplayer part.
- The multiplayer connection is run on the run_client() thread which uses some socket programming to constantly send and receive game state updates.

There are no known bugs, although we did not get to implement the Faisal texture for the fourth player due to some difficulties with the windows/unix systems networking code. We hope you enjoyed our project!

### Networks

The "cmd/networks" directory contains the server part of the project. After a significant attempt to include gRPC to communicate between a go server and a c++ client, we pivoted to an idea of just creating a UDP based server. 
The server continuously receives UDP packets with an expected byte layout. The server then checks if player had a registered id yet, and if not, registers the player and provides back an id. Then, every time a player sends an update, the server udpates the player's information, and sends it to other players as they query it. 
The core of the project just focused on keeping the data organized, completing a UDP packet read loop, and providing code for Marshalling/Unmarshilling player and game information. 

The client side, located in realtime.cpp, sends the first packet as an unregistered player with a -1 id, and after receiving a proper id, sends the server information about their own position and/or hitevents, as well as reads updates from the server to update other players positions on their local copy. 
