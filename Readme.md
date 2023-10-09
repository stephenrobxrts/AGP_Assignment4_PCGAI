#Uni  #U41024  #Unreal #CPP  #Assignment 


## Rollercoaster ideas

You are on a rollercoaster track (Spline, PCG, Caves?) 
Shoot other player  (other rollercoaster? Flying enemies)

Can Duck, shoot, other paths?

## Procedural Jones: Raiders of the Randomly Generated Relics

Get into the cave system, find switch(es), escape

(Place collected relics on to specially shaped pedestals to open exit)

#### PCG
* FBM Terrain
* Cave System
	* Marching Squares Cave System
		* Blocked out rooms with pcg terrain
		* Inner room details? 
* PCG Pathfinding
	* Node differentiation by doorway/terrain/InCanve(x)
	* Jump Point Search (instead of A*)
		* <iframe width="560" height="315" src="https://www.youtube.com/embed/kSm-ADXH808?si=Lr-eFDDqkvvqzi3p" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>
	* 

#### AI
* AI Hearing (head to doorway of rooms if heard elsewhere -> *investigate*)
	* Which side of room is player on? 
* Predict movement of player towards goal?
	* Traps?
* Multiple AI
	* +Communicate position, route
* ++Dynamic Lights (torches?)?
	* Player Placed/Pickup torches
		* Inventory?
* Smokescreen (AI/Player?)

#### Bonus
* Minecart escape sequence (hehe)
	* Splines
	* Explosions


## Implementation

### Cave generation
Inputs:
* LevelSize
* HeightDifference (The difference between the start point and the end point)
* Cave min/max sizes
* Tunnel size
* Number of paths (min 2)
* Interconnection ratio (at 1 it generates ALL possible interconnects)

Places Rooms 
Level is given a size
**WRITE MORE FOR THE VIDEO**




## A3 Objectives

### PCG
- [ ] Level Generator
	- [x] Makes Start/End Rooms ✅ 2023-10-09
	- [x] Makes Rooms that follow paths inbetween ✅ 2023-10-09
	- [x] Makes Tunnels to connect the path ✅ 2023-10-09
	- [x] Makes Random Interconnects ✅ 2023-10-09
	- [ ] Room NavNodes
	- [x] Send RoomData to Marching Chunks ✅ 2023-10-09
- [ ] Rooms
	- [x] Boxes with (optional) lights ✅ 2023-10-09
		- [ ] Light attached to wall
		- [ ] LightEnabled Variable
		- [ ] Light as a PickupObject
	- [x] Random size ✅ 2023-10-09
	- [ ] Room NavNode
	- [ ] Walkable NavNodes
	- [ ] Traps
- [ ] Tunnels
	- [ ] Connects Box/Box
	- [ ] Correctly rotated floor
- [ ] Marching Cubes
	- [x] Correctly shows boxes/tunnels ✅ 2023-10-09
		- [ ] Fix Seam Issue
	- [x] Noise function ✅ 2023-10-09
		- [ ] FBM noise
	- [x] Working ChunkSize/VoxelDensity fields ✅ 2023-10-09
	- [x] SDF for inside/outside ✅ 2023-10-09
		- [x] SDF interacts with noise ✅ 2023-10-09
- [ ] Chunks
	- [x] Spawn chunks only where boxes/tunnels are ✅ 2023-10-09
		- [x] Boxes ✅ 2023-10-09
		- [ ] Tunnels
	- [ ] Octree or similar chunk optimization
		- [ ] Change ChunkSize OR Voxel Density?
		- [ ] Switch out geometry w/o disrupting actors

### AI


## A4 Plan

