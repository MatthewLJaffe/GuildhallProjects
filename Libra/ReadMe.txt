Controls:

Keyboard
Move Tank WASD
Move Turret IJKL
Fire bullet Space
Fire flamethrower Left Shift

Controller 
Move Tank Left stick
Move Turret Right stick
Fire bullet Right trigger
Fire flamethrower Left trigger

Debug
Speed up time Y
Slow down time T
Debug mode F1
No damage F2
No clip F3
Map camera F4
Cycle heat map draw F6
Hard restart F8
Switch Maps F9

Known bugs:

My UV corrections are not working. I am confident that I am offsetting the UV mins and maxs by 1/128th of a texel like we said to do in class but I am still getting little lines on my tiles.

Destructible tiles are implemented but the map will still be regenerated if destructible tiles are blocking all paths to the exit as they are treated like solid tiles. I'm also leaking memory in some places.

Enemies get stuck on eachother in 1 tile wide corridors and are unable to reach their waypoints