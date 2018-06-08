Hello,
This is a reproduction of the classic game Snake. The objective of the game is that the snake has to eat the target fruit, which makes it grow in length. 
As the snake eats the fruit, it disappears, and another one appears at a random location.
The snake can die by eating itself (when it collides with itself) or by hitting the edge of the screen.

The controls for moving the snake is [w] for UP, [a] for LEFT, [s] for DOWN and [d] for LEFT. The game meets all the basic requirements specified in the assignment. 
You can use [p] to pause the game anytime you want, [r] to restart the game and [q] to quit the game altogether.
As for my design decisions, I have the entire body of snake including its head 20x20 px in dimensions. The snake moves 20 px(defined as a block) at a time. 

While starting the game you can pass 2 arguments. First is the FPS of the game which is defined as "how often the screen is repainted in 1 second" and second is speed of the snake which is defined as "how fast the snake moves".
The exact way how i define speed is number of blocks the snake moves in 1 second. So basically the snake stays on one block for some number of frames. That can be easily calculated by the formula:
# of frames that snake stays on a block = FPS / Speed.

To make the game a bit more playable, I internally double the speed inside the game otherwise the snake just moves too slow. 

Score is calculated as 
Score = Number of fruit eaten * Speed passed to the game.

Good luck have fun playing the game!! 
Thank you so much for playing the game ^ _ ^
