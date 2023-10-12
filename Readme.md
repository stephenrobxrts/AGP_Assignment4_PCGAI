## Procedural Jones: Raiders of the Randomly Generated Relics

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
![Pasted image 20231012104255](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/68069a4d-9c1f-403e-b66d-706d4273359d)
![Pasted image 20231010164352](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/be371226-3459-457f-a9ca-cc606e53f113)

You can also set it to show the RoomNodes, low LOD navNodes that will allow for efficient pathing across a large level. 
![Pasted image 20231012104206](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/f95c9644-9193-46c1-9e00-8192011e9f1f)


In Chunks you can show Chunk Borders and show every voxel
![Pasted image 20231010164431](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/e6e352f9-27d5-4779-b80e-cbb066af2b39)
![Pasted image 20231010164604](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/4dd73271-4cb8-4848-9b72-89232389d541)


```ad-warning
Do not turn on DebugVoxels unless DebugOnly1Chunk is ON and VoxelDensity is less than 12. 
```

You can also invert the solids to make the cave visible from the outside, which has a corresponding impact on the voxel values 
![Pasted image 20231010164727](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/45bbfb00-dbcf-4ac7-a017-7b387cacc488)


(Invert solids with a directional light in the scene)
![Pasted image 20231010164847](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/dfc81642-120f-417e-ac4b-8ea137417d9c)

### Level Generator Controls

#### Inputs:
* LevelSize
* HeightDifference (The difference between the start point and the end point)
* Cave min/max sizes
* Number of paths (minimum 2)
* Approximate number of boxes per path (in diagram below this is 4)
* Interconnection ratio (at 1 it generates ALL possible interconnects)
![Pasted image 20231010120934](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/7ea215e1-8dc0-4b87-b27b-877c40469a2b)


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
![Pasted image 20231012164539](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/3b486113-39a8-4d96-b7bc-d74ff782b652)



##### Interconnections
We then generate interconnects based on the **Interconnectedness** variable. Interconnections are like rungs on a ladder, they connect a box from one path to another. If the variable is 1, it will try to create all possible interconnections. 
A connection is deemed "possible" if it doesn't intersect with another box, and the gradient isn't so steep you can't walk down it. 

High vs Low interconnectedness
![Pasted image 20231010172846](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/d41c1357-46c0-4fff-b453-4a1d975f5b1c)
![Pasted image 20231010172902](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/0a6e98e6-42f9-4f64-b189-c08f39153dc3)





#### Chunk Creation
The system then generates chunks within the world. A chunk is just a small section of our cave - I'll detail why we need chunks later. 
A chunk is only created if it intersects with one of the boxes or tunnels (+ a small margin of error). Because the chunk collision check includes tunnels (and their rotation) the check for intersection is a little slow. 
Only the voxels inside a chunk will be checked during marching cubes, this gives us a good way to increase efficiency. I've found 256, **512**, 1024 to be good values. 


1024 sized chunks
![Pasted image 20231010174157](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/6cae2a5b-f760-4665-a693-e88809d8e89c)

512
![Pasted image 20231010174213](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/21919075-5d52-40bb-8641-cfd83bd3122f)

256
![Pasted image 20231010174249](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/59c0a2f2-e171-4056-9743-a28f79f8ea1e)




### Marching Cubes

The object data (all objects that will interact with the voxel fields, and the chunks to be meshed) is passed to the marching Cubes Terrain mesher. 

```ad-todo
This could be sped up by only passing objects that are in the same area as the chunk. Spatial hash functions or an additional input (based on the overlap test)
```


#### Voxels 

Given the level boxes (rooms, tunnels) that make up the cave, we now need a way to generate geometry, in such a way that we can add noise and distort the shapes so we don't just end up with... cubes. 

To do this we sample the space at discrete locations, these voxels (volumetric pixels) will check their distance to every object in the array, subtracted by the size of the object in the direction of the voxel. 

##### Signed Distance Fields

![Pasted image 20231012154726](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/7cbc74c2-d07b-413e-aceb-6d32721cbc83)

![Pasted image 20231012155020](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/73564014-90c9-4e51-99ae-01b5499d8a34)

![Pasted image 20231012155146](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/90aaa7c7-d28b-4472-ba3c-260396d483df)


In this scenario we are directly parallel with the length, but if the voxel is not parallel with any major axis of the box then it needs to be the sum of the components in that direction. 

The tunnels are also rotated, to check for that we just unrotate everything first. (Multiply by inverse quaternion)

If the object is outside the object, it will return a positive number, if it is inside it will return a negative number. If it lies exactly on the surface the value will be 0. 

#### The algorithm 

So we have a bunch of boxes in an area... we need to create our geometry.

Creating geometry is easy, we've done that in the Procedural Landscape tutorial - so a lot of this will be copy pasted from that. 
But that created a 2D plane, we need a lot of 3D geometry to make our caves. 

We'll use an algorithm called Marching Cubes to do this. The premise is surprisingly simple. If we have some random boolean noise (1 or 0) that we sample at 4 points then we could make a square out of those 4 points, drawing a line between the midpoints of the square where the values are opposite.
![Pasted image 20231012155146](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/a84e508f-3403-4bea-837e-43f5eaed4ac0)


This idea scales up to 3D as well, instead of a line it will generate a rectangle, which we can split to make 2 triangles. 

If the values are not 0 or 1, but instead
-1 and 1 then our midpoint line lies where the field is 0. 

Lastly, if the values are not boolean but instead some float between
-1 and 1 then we can draw the line only if they have opposite signs, and bias it towards the smallest (absolute) value. 

So if we look at this example again
![Pasted image 20231012153739](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/288b6227-87bc-4943-8c86-59027b987dc2)

If a filled circle = 1.0 and unfilled = -1.0 the above is correct. 
The points are the maximum difference apart they could be: **2.0**.

But if the filled circle is only 0.2 then the points lie only 1.2 units apart. The line will be drawn 80% of the way towards the filled circle *from the midpoint between it and the unfilled circles*

We can march this over the voxels we created earlier and make the rooms!

![Pasted image 20231012155433](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/5bf9491c-386c-4efd-9004-94562eb00c2b)


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
![Pasted image 20231012151325](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/d948857f-575f-404d-b415-cf9ec7bb10f6)

Achieving this sort of resolution over the entire cave requires 32 voxels per side length of chunk. The debug image earlier had 8.
![Pasted image 20231012151444](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/dd2674e0-0037-442a-a44a-d479963582d5)
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
The level already performs well. It is capped at 60fps in all scenarios I can throw at it. 
But there is definitely room to optimize the loading times.

A few ToDo's above are in reference to optimization. Beyond that I've set up the system to behave well if you switch out chunks (either for smaller chunks, or higher voxel densities). The framerate is already great, even at high resolutions like 
512x ChunkSize 32x VoxelDensity
![Pasted image 20231012170341](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/87811213-6941-474e-90a7-fe63230a06ba)



But more importantly it would allow us to speed up the loading times and RAM used during level generation (which gets to 16gb at 512x Chunk and 64x Voxel Density)
![Pasted image 20231012170646](https://github.com/Peregrine777/AGP_Assignment3_PCGAI/assets/111720817/54f2e65c-7d36-4ce3-871c-d3a762b4d8e9)


I anticipate using Oct-Trees for this, but have not experimented with it enough to see if switching out parts of a procedural mesh will play nice with the player/enemy collisions or any physical objects. Those may have to be cached and spawned when a player is close enough to load in the LOD0 caves (best quality). 
I also aim to investigate whether compute shaders can be used in Unreal Engine - I am competent at writing shaders but unfortunately they weren't implemented on the other software I'm used to so I'd like to explore that area more.

#### Interactions

A core part of this will be having the player interact with parts of the levels. We anticipate adding: 

* **Hiding Spots:** Small alcoves covered by vines with no AI navNodes inside. Players can hide in these to escape -> But if an enemy spots you going in to one they can brush aside the vines and catch ya! 
* **Doors & Triggers:** It's not Procedural Jones unless he narrowly escapes under a Low-Poly Door. A basic mesh in some rooms near the tunnel entrance that blocks progress (or can be shut to seal out the enemy)
* **Artefacts/Keys:** The last room is currently the end of the map, it should be the trigger point for keys that are found throughout the map - when the keys have been gathered the final door opens leading to the golden altar!
* **Traps:** It's easy to make holes in the mesh, these pits will not have AI paths over them to stop them from falling in. But you could be able to place wooden squares over the top which makes the mesh appear to exist but with a trigger box that gets rid of it (and a brushed dirt texture so players don't fall into it themselves). 
* **Interactive torches:** Some rooms will spawn without lights, they player should be able to light/snuff torches that are on the walls and carry a torch with them. Lights are great for seeing what you're doing, but a dark room prevents you from getting caught!
* **AI Spawn points** while setting these are easy (the same logic as the PlayerStart Position) we need to think through where we should place them

##### Putting it together
Given the other sections get completed, it would be very nice to have this as a playable experience with AI Voice lines (like the F.E.A.R AI Whitepaper) that indicate lots of states. I have found that recording the voicelines and passing them through AI Voice imitation is great for this, and it gives the player an indication of AI state while playing that really adds to the experience. 

With the addition of a second player, the concept of kiting enemies, distracting and noise-makers would be amazing to add. 

And... well after taking the idol at the end of the level generating a run area that is linear but timed (with something chasing you) or a spline-based minecart escape would really add to the experience. Because I expect this area will be separated with a door it can be loaded (and the main section unloaded) out of the players' sight. 


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

