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
