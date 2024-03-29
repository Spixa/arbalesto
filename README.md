# arbalesto
2d videogame

## Current features
* Game/Menu state
* Server/Client interface
* Sprites and hitboxes
* Resource manager
* Animations

## Animation rendering
I use a made up technique to render each sprite. Each sprite has four animation "layers", this includes feet, body, hand, and item. all 4 layers are the same size and overlay eachother in different orders. This allows for the feet to for example run while the body and hand are playing the item holding animation and the item animation is playing let's say a sword holding animation.

## Server/Client interface
The server/client interface uses the [openSIMP v1](https://github.com/spixa/openSIMP) schema. For now the server/client interface is synchronous and manages reception and connection on seperate threads.

## Plans
* Network support for movement and interactions and whatnot
* Menu state with GUI
* Collision boxes and physics

## Collision detection and physics
This is yet to be implemented in the game. But for now I just want to preface how I want it to be implemented. I think it would be the best option if collision detection is predicted clientside and made the player controlled by the client not collide with objects client side, but then also be corrected when it is calculated incorrectly by the client, on the serverside. This could also prevent cheaters from turning off collision on their client but being met by the server's collision correction. I would also like to make this feature togglable as an "Anticheat" measure that could be turned off  

## UI
The UI of this game will be pretty simple to use. Anything related to the player itself like the inventory, skills and shop UI will be full-screen and take up the entire screen. If a certain area a player is in or a certain clickable area has a UI it will be displayed above or at a preset location relative to that area

## Buildings
The game is most likely gonna be a side-scroller, and the buildings will be built from two textures, one is the outside texture and one the inner texture with defined collisions and utilities inside the building. When players are outside of a building the only layered is the outside one, once the player gets inside a building, they can start to see the inside