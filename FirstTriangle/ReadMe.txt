How to Use:

Game controls:

Controller:
Left stick: rotate playership / boost
A Button: Fire bullet
Start Button: Respawn
Right Trigger: Fire missiles
Left Trigger: Fire laser

Keyboard:
S: rotate left
F: rotate right
E: boost
SPACE: fire bullet
N: Respawn
W: fire laser beam
R: fire missiles
Escape: Quit to menu / quit game

Engine controls:
F1: Developer Mode
F8: Hard reset
T: 1/10th speed
O: Step 1 frame
P: Pause / unpause



Known Issues:
I noticed a crash that occoured when exiting the game in release mode one time. However I have been unable to reproduce this bug. Asteroids are not damaged by laser beam. This is intentional becuse asteroids are meant to act as cover from enemy bullets and I did not want them to be wiped out too fast but this is probably unintuitive. Pressing the I key doesn't spawn asteroids anymore. I wasn't sure if we were supposed to keep this feature but it made my wave spawning more complicated so I removed it.

Deep learning

I would say that my approach to coding for this project could be broken down into the following steps. First I would define the problem. This may sound simple, but when working on something with multiple moving parts, taking a second to list out the requirments on a piece of paper was immensely helpful to me in figuring out what to prioritize. After this I would make whatever files I thought I would need start by writing out the main functions I thought I would need in the header files. I have learned to try to not spend too much time in advance thinking about what functions will be required as it is much more natural for me to come up with the lower level functions I will need while working on the higher level ones. I would describe this approach as a funnel since I am starting with the broadest and most general understanding of what I need to do and narrowing down as I go. One thing I learned in class that I am still trying to apply is starting with the simplest version of something, passing unbiased judgment on it and then only going back and iterating if it is nececary. I have found that I get very attached to the idea of how something will work in my head that I don't stop to think about if there is a simpler version that is a better use of my time. One example of this is the trail animations on the missiles. the entire trail is drawn from one array of verticies manually. Creating a dynamically curvey trail from triangles was making the problem more complicated than it needed to be in order to get a cool trail animation. In retrospect I think that the simplest thing I could've done is continally spawn pieces of debris behind the missiles as they move. This would've taken 10% of the time and probably would've been 80% as good. In the future when I find myself writing a piece of code that is overly complicated, I will try to simplify it as much as possible and then only go back and add complexity if I feel it is lacking in what I want to convey. 



 