![Pasted image 20231012104255](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/68069a4d-9c1f-403e-b66d-706d4273359d)## Procedural Jones: Raiders of the Randomly Generated Relics

![Pasted image 20231012151325](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/d76dba22-63a7-48f7-bdf0-1706b5d6add0)


```ad-abstract
Get into the cave system, find switch(es), escape

(Place collected relics on to specially shaped pedestals to open exit)
```

#### PCG
* FBM Terrain
* Cave System
	* Marching Squares Cave System
		* Blocked out rooms with pcg terrain
		* Inner room details? 
	* **++Hidey Holes** 
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


## PCG A3 - Sam Mckenzie-Sell


For assignment 3 I have worked on a procedural cave generation system that combines a level generator that places rooms and tunnels and a marching Cubes system that intelligently creates chunks based on the level. 

The system uses deterministic algorithms to generate the cave and mesh, making replication of the level (which can be too large to save) light on performance. 

To interact with the level go to Levels, SamTests, ProceduralMap. 

The AProceduralCaveGen Actor contains the controls for the generation. These controls are arranged in to sections for the CaveGen, Chunks and Noise
![Pasted image 20231010163948](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/1c3a2bff-599e-4fa3-a518-0df90b4b9b3c)


By default I have set this up to generate simple test level that is light on performance.
### Debug options

Debug options are at the bottom of each section. 
In PCG You can toggle Showing the rooms/tunnels in realtime and set it to only render 1 chunk instead of the whole level.
![Pasted image 20231012104255](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/99c5ac0b-7014-4df2-af80-d0c70fcaaf3e)
![Pasted image 20231010164338](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/2cfbf221-c324-488b-9848-2a38e3472cad)
![Pasted image 20231010164352](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/381b6aa1-44d9-447f-b0fe-87e6bde136ae)


You can also set it to show the RoomNodes, low LOD navNodes that will allow for efficient pathing across a large level. 
![Pasted image 20231012104206](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/60a0e926-f8f0-4b4d-9b48-d8d2aec3df2a)


In Chunks you can show Chunk Borders and show every voxel
![Pasted image 20231010164431](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/f4181951-7cb3-4f77-9b21-7c98dc233310)
![Pasted image 20231010164604](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/8bc4d1bf-b12c-4b6c-b4be-4dbd7c84febd)

```ad-warning
Do not turn on DebugVoxels unless DebugOnly1Chunk is ON and VoxelDensity is less than 12. 
```

You can also invert the solids to make the cave visible from the outside, which has a corresponding impact on the voxel values 
![Pasted image 20231010164727](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/d6b0ee84-156a-46b1-9a88-316422e024bb)


(Invert solids with a directional light in the scene)
![Pasted image 20231010164847](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/a59cd839-8eea-400a-8983-d8e987c5b49d)

### Level Generator Controls

#### Inputs:
* LevelSize
* HeightDifference (The difference between the start point and the end point)
* Cave min/max sizes
* Number of paths (minimum 2)
* Approximate number of boxes per path (in diagram below this is 4)
* Interconnection ratio (at 1 it generates ALL possible interconnects)
![Pasted image 20231010120934](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/93d5ed53-c0ea-42a6-9f0c-f8983ebe2a2e)


#### Operation
The Cave generation system lays out the cave system using a series of boxes. 
A starting box is placed at  0, 0, 0
An ending box is placed at  Levelsize, 0, 0

The distance between start/end will be filled by approximately 'numBoxesPerPath' boxes
and there can be more than one path
So we go towards the destination, but with some accuracy smudge factor, by approximately that distance and place a box
The box we just placed is then linked to the box before it on the path. 

##### Room Creation
When a box (room or tunnel) is placed it also runs some code that can be used to spawn items. At the moment this just generates a light on the inside of the rooms.

```ad-todo
This will be used to randomly spawn enemies, torches (for the light sources) and some traps
```


##### Tunnel Creation
A tunnel is just a room but with a start and end box defined. 

Tunnels are rotated and sized to face the destination box, they are also offset from the center of the box to stop tunnels being made above head height. 

Offsetting the tunnels also makes the different paths clear to the player
![Pasted image 20231012164539](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/dfc689d6-5d21-454f-bbe8-864506a3c9f7)



##### Interconnections
We then generate interconnects based on the **Interconnectedness** variable. Interconnections are like rungs on a ladder, they connect a box from one path to another. If the variable is 1, it will try to create all possible interconnections. 
A connection is deemed "possible" if it doesn't intersect with another box, and the gradient isn't so steep you can't walk down it. 

High vs Low interconnectedness
![Pasted image 20231010172846](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/e6197f15-c97f-41f3-9e66-b7ab86c258c2)
![Pasted image 20231010172902](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/531412e3-86fb-4cd6-a9a4-f7a1c1908763)





#### Chunk Creation
The system then generates chunks within the world. A chunk is just a small section of our cave - I'll detail why we need chunks later. 
A chunk is only created if it intersects with one of the boxes or tunnels (+ a small margin of error). Because the chunk collision check includes tunnels (and their rotation) the check for intersection is a little slow. 
Only the voxels inside a chunk will be checked during marching cubes, this gives us a good way to increase efficiency. I've found 256, **512**, 1024 to be good values. 


1024 sized chunks
![Pasted image 20231010174157](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/1d77f108-1c3e-4d53-b6f0-0fe3c86ce00d)

512
![Pasted image 20231010174213](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/7c3eadf1-0084-41b8-9e47-3b8e1d0b60a2)

256
![Pasted image 20231010174249](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/bd63c384-3829-4b7e-bf94-20a3aaf62905)




### Marching Cubes

The object data (all objects that will interact with the voxel fields, and the chunks to be meshed) is passed to the marching Cubes Terrain mesher. 

```ad-todo
This could be sped up by only passing objects that are in the same area as the chunk. Spatial hash functions or an additional input (based on the overlap test)
```


#### Voxels 

Given the level boxes (rooms, tunnels) that make up the cave, we now need a way to generate geometry, in such a way that we can add noise and distort the shapes so we don't just end up with... cubes. 

To do this we sample the space at discrete locations, these voxels (volumetric pixels) will check their distance to every object in the array, subtracted by the size of the object in the direction of the voxel. 

##### Signed Distance Fields

![Pasted image 20231012154726](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/2f2cd463-3470-4774-89c3-10c6c318e1aa)

![Pasted image 20231012155020](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/5905b361-ab81-4dce-9565-3b23eb8fad5c)

![Pasted image 20231012155146](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/6c46c0b3-4906-4504-942c-a576435238c0)


In this scenario we are directly parallel with the length, but if the voxel is not parallel with any major axis of the box then it needs to be the sum of the components in that direction. 

The tunnels are also rotated, to check for that we just unrotate everything first. (Multiply by inverse quaternion)

If the object is outside the object, it will return a positive number, if it is inside it will return a negative number. If it lies exactly on the surface the value will be 0. 

#### The algorithm 

So we have a bunch of boxes in an area... we need to create our geometry.

Creating geometry is easy, we've done that in the Procedural Landscape tutorial - so a lot of this will be copy pasted from that. 
But that created a 2D plane, we need a lot of 3D geometry to make our caves. 

We'll use an algorithm called Marching Cubes to do this. The premise is surprisingly simple. If we have some random boolean noise (1 or 0) that we sample at 4 points then we could make a square out of those 4 points, drawing a line between the midpoints of the square where the values are opposite.
![Pasted image 20231012153739](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/d5211b5b-268b-4ae0-9280-704a818b7a81)


This idea scales up to 3D as well, instead of a line it will generate a rectangle, which we can split to make 2 triangles. 

If the values are not 0 or 1, but instead
-1 and 1 then our midpoint line lies where the field is 0. 

Lastly, if the values are not boolean but instead some float between
-1 and 1 then we can draw the line only if they have opposite signs, and bias it towards the smallest (absolute) value. 

So if we look at this example again
![Pasted image 20231012153739](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/fbf72c6e-dfaf-4c02-a071-64dd2229b1b7)

If a filled circle = 1.0 and unfilled = -1.0 the above is correct. 
The points are the maximum difference apart they could be: **2.0**.

But if the filled circle is only 0.2 then the points lie only 1.2 units apart. The line will be drawn 80% of the way towards the filled circle *from the midpoint between it and the unfilled circles*

We can march this over the voxels we created earlier and make the rooms!

![Pasted image 20231012155433](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/5aba5644-22cb-4c8a-91a4-e0e2f37ca476)


```ad-note
I'm using a bunch of precalculated functions that define how different patterns of 1's and 0's for each of the vertices of the box will result in triangles. These have been adapted from Sebastian Lague's brilliant Marching Cubes video
```
#### Adding Noise

For the Procedural Landscape we used 2D Perlin Noise, I'm just using the same thing but in 3D from FastNoise (MIT license).

Originally the voxels were calculated just by checking if they were inside, if they were they had a value of 1, otherwise -1. But if you introduce noise into that you end up with either no effect or bits of rock hovering in your face so you can't walk anywhere. Switching to the Signed Distance Function (SDF) means that we can scale the noise based on how close the original voxel was to a surface. So we can have maximum impact on the walls themselves, without clogging up the interior of the room. 

We scale the SDF so that it reaches a maximum at about 1 human's height (to try to ensure that no room is crushed too small to walk through). The noise will output values between -1 and 1, so all we need to do is multiply the noise by 1/abs(voxelValue) and it will impact the walls

```ad-todo
The noise is currently very high frequency (lots of small geometry) but I'd like it to have a mixture of frequencies using fractal brownian motion.
```

#### Chunks
Say that we want a reasonable resolution for our cave:
![Pasted image 20231012151325](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/da5b034b-f83c-45d1-89e2-dc9ae2af054b)

Achieving this sort of resolution over the entire cave requires 32 voxels per side length of chunk. The debug image earlier had 8.
![Pasted image 20231012151444](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/f2ba62a2-1e67-4d23-9c3c-bac408f236e8)
Therefore each chunk will contain 32^3 voxels. 
The test level I'm using is about 10,000 units in length, and at a chunk size of 1024 that means it will be around 10 chunks long. If we're not using chunks that means we need 320^3 voxels to make a cube shaped play area that fits the level. 

And for each voxel we aim to calculate the (signed) distance between that voxel and the nearest object. (looping through all objects)

This is... too much. Unreal Engine crashes upon trying it.

Instead we will use the chunks to divide up the level, and only draw chunks where they overlap with a box. That drastically reduces our checks. The smaller we make our boxes, the faster this whole thing will run - up to a point. If we made our chunks the size of a single voxel (each containing a single voxel) then we will have just added overhead for no gain. 

Note that the box checks can't be the super fast ones I was using for room placement, they have to take into account the rotation of the destination box (in the case of tunnels). Without this there will be holes in some of the tunnels. 

##### Voxels / Chunk

Earlier I stated that the debug image of voxels had 8 per side, but actually there are 9. 
We've added one extra voxel per dimension which should be the same as the first voxels for the next chunk. We need both chunks to stitch together so by calculating that we get the geometry that covers that last bit of distance.

```ad-question
There are still seams in between some geometry if you look carefully - it probably has to do with this step, float precision and powers of 2. 
Can you figure out why this is happening?
```

### A4 Plan


#### Initial steps
The few remaining ToDo's in PCG need to be done
++ Indicates extras that I may come back to. 
Especially placing navNodes inside this custom geometry. Actors can path between rooms but we do need more detail than that. 

The AI side of things should be integrated, and any missed core objectives there must be implemented. AI will need some extra checks for finding alternative pathways if a certain route is blocked. Ideally they can notice that a route is blocked and (after trying to get through it for a bit and not succeeding) get rid of the node->node connection they thought was valid. It would also be great to implement jumping and crouching for the enemy.

#### Optimization
A few ToDo's above are in reference to optimization. Beyond that I've set up the system to behave well if you switch out chunks (either for smaller chunks, or higher voxel densities). The framerate is already great, even at high resolutions like 
512x ChunkSize 32x VoxelDensity
![Pasted image 20231012170341](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/1ba744cb-c1e5-45c3-bf81-6a0bedc1ddf4)



But more importantly it would allow us to speed up the loading times and RAM used during level generation (which gets to 16gb at 512x Chunk and 64x Voxel Density)
![Pasted image 20231012170646](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/cf9d3e76-4116-46fb-b834-58ebb27ef1ca)


I anticipate using Oct-Trees for this, but have not experimented with it enough to see if switching out parts of a procedural mesh will play nice with the player/enemy collisions or any physical objects. Those may have to be cached and spawned when a player is close enough to load in the LOD0 caves (best quality). 
#### Interactions

A core part of this will be having the player interact with parts of the levels. We anticipate adding: 

* **Hiding Spots:** Small alcoves covered by vines with no AI navNodes inside. Players can hide in these to escape -> But if an enemy spots you going in to one they can brush aside the vines and catch ya! 
* **Doors & Triggers:** It's not Procedural Jones unless he narrowly escapes under a Low-Poly Door. A basic mesh in some rooms near the tunnel entrance that blocks progress (or can be shut to seal out the enemy)
* **Artefacts/Keys:** The last room is currently the end of the map, it should be the trigger point for keys that are found throughout the map - when the keys have been gathered the final door opens leading to the golden altar!
* **Traps:** It's easy to make holes in the mesh, these pits will not have AI paths over them to stop them from falling in. But you could be able to place wooden squares over the top which makes the mesh appear to exist but with a trigger box that gets rid of it (and a brushed dirt texture so players don't fall into it themselves). 
* **Interactive torches:** Some rooms will spawn without lights, they player should be able to light/snuff torches that are on the walls and carry a torch with them. Lights are great for seeing what you're doing, but a dark room prevents you from getting caught!
* **AI Spawn points** while setting these are easy (the same logic as the PlayerStart Position) we need to think through where we should place them


## A3 Objectives

### PCG
- [x] Level Generator 
	- [x] Makes Start/End Rooms 
	- [x] Makes Rooms that follow paths inbetween 
	- [x] Makes Tunnels to connect the path 
	- [x] Makes Random Interconnects 
	- [x] Send RoomData to Marching Chunks 
- [x] Marching Cubes 
	- [x] Correctly shows boxes/tunnels 
		- [ ] ++Fix Seam Issue
	- [x] Noise function 
		- [ ] ++FBM noise
	- [x] Working ChunkSize/VoxelDensity fields 
	- [x] SDF for inside/outside 
		- [x] SDF interacts with noise 
- [ ] Rooms
	- [x] Boxes with (optional) lights 
		- [ ] Light attached to wall
		- [ ] LightEnabled Variable
		- [ ] Light as a PickupObject
	- [x] Random size 
	- [x] Room NavNode 
	- [ ] A4 - Walkable NavNodes
	- [ ] A4 - Traps
	- [ ] A4 - Doors
- [x] Tunnels 
	- [x] Connects Box/Box 
	- [x] Correctly rotated floor 
- [x] Chunks 
	- [x] Spawn chunks only where boxes/tunnels are 
		- [x] Boxes 
		- [x] Tunnels 
	- [ ] ++Only check against boxes we know overlap
	- [ ] ++Octree or similar chunk optimization
		- [ ] Change ChunkSize OR Voxel Density?
		- [ ] Switch out geometry w/o disrupting actors

### AI


#### Modify Classes

* [ ] Rooms
	* [ ] RoomNode (points to connected Rooms)
	* [ ] NavNodes (Some navnodes are "doorway nodes" and point to another room)
* [ ] AI Hearing (head to doorway of rooms if heard elsewhere -> *investigate*)
	* [ ] Which Room sound was heard from
	* [ ] Which Path is player 
	* [ ] ++Predict movement of player towards goal?
* [ ] AI State
	* [ ] Investigate
	* [ ] Ambush
* [ ] Attempt to jump if blocked by geometry
	* [ ] If progress towards goal not going up, try jump
	* [ ] Try crouch
	* [ ] ++ Break node->node connection and pathfind again
* [ ] Handle player behaviour
* [ ] Basic Sounds for testing

