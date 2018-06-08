/*
Commands to compile and run:

    g++ -o snake snake.cpp -L/usr/X11R6/lib -lX11 -lstdc++
    ./snake

Note: the -L option and -lstdc++ may not be needed on some machines.
*/

#include <iostream>
#include <sstream>
#include <list>
#include <cstdlib>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <string>
/*
 * Header files for X functions
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;
 
/*
 * Global game state variables
 */
const int Border = 1;
const int BufferSize = 10;
const int width = 800;
const int height = 600;
int FPS = 30;
int Speed = 5;
int fruitX;
int fruitY;
int Score = 0;
bool Up = false;
bool Down = false;
bool Right = false;
bool Left = false;
bool FruitEaten = false;
bool GamePaused = false;
bool GameOver = false;
bool GameStart = false;
bool StartSplashScreen = true;
/*
 * Information to draw on the window.
 */

void GameEnd() {
  Up = false;
  Down = false;
  Left = false;
  Right = false; 
  GameStart = false;
  GameOver = true;
  GamePaused = false;
  StartSplashScreen = false;

}

string convertInt(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

/*
 * Function to put out a message on error exits.
 */
void error( string str ) {
  cerr << str << endl;
  exit(0);
}


struct posn {
  int xcord,ycord;
  posn(int x, int y): xcord(x), ycord(y){};
};

struct XInfo {
  Display  *display;
  int    screen;
  Window   window;
  GC     gc[3];
  int   width;    // size of window
  int   height;
};


/*
 * An abstract class representing displayable things. 
 */
class Displayable {
	public:
		virtual void paint(XInfo &xinfo) = 0;
};       

class Snake : public Displayable {
	public:
		virtual void paint(XInfo &xinfo) {
			for (int i = 0; i < body.size(); ++i){
				XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], 
						body[i].xcord, body[i].ycord, 20, blockSize);
			}
		}

		void move(XInfo &xinfo) {
			if (Up) {
				y = y - Ydirection;
			}
			else if (Down) {
				y = y + Ydirection;
			}
			else if (Right) {			
				x = x + Xdirection;
			}
			else {
				x = x - Xdirection;
			}
			//y = y + Ydirection;
			for (int i = body.size()-1; i >= 1; --i) {
				body[i].xcord = body[i-1].xcord;
				body[i].ycord = body[i-1].ycord; 
				//printf("coordinate %d: %d, %d\n",i+1, body[i].xcord, body[i].ycord);
			}
			body[0].xcord = x;
			body[0].ycord = y;
			//printf("coordinate of head %d: %d, %d\n",1, body[0].xcord, body[0].ycord);

			didHitObstacle();
			didEatFruit();
		}

		int getX() {
			return x;
		}

		int getY() {
			return y;
		}

		void didEatFruit() {
			if (x == fruitX && y == fruitY) {
				int sizeSnake = body.size()-1;
				body.push_back(posn(body[sizeSnake].xcord,body[sizeSnake].ycord));
				Score = Score + Speed/2;
				//printf("Score is: %d", Score);

				FruitEaten = true;
			}
		}

		void didHitObstacle() {
			if (x < 0 || x+20 > width || y < 0 || y+20 > height) {
				GameEnd();
			}
			for (int i=1; i < body.size(); ++i) {
				if (x == body[i].xcord && y == body[i].ycord) {
					GameEnd();
					break;
				}
			}
		}

		Snake(int x, int y): x(x), y(y) { //x,y are tracking x and y coords of the top left of snake head 
			Xdirection = 20; //movement jump in x direction
			Ydirection = 20; //movement jump in y direction
			blockSize = 20; // length of a block, height of a block is predefined to be 20
			body.push_back(posn(x,y)); //adding first body part
			body.push_back(posn(x-blockSize, y)); //adding second body part
			body.push_back(posn(x-2*blockSize, y)); // adding third body part

			//printf("coordinate 3: %d, %d\n", x-2*blockSize, y); 
			//printf("coordinate 2: %d, %d\n", x-blockSize, y);
			//printf("coordinate 1: %d, %d\n", x, y);
		}

	private:
		int x;
		int y;
		//int xEnd;
		//int yEnd;
		int blockSize;
		int Xdirection;
		int Ydirection;
		vector <posn> body;
};

class Fruit : public Displayable {
	public:
		virtual void paint(XInfo &xinfo) {
			XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], x, y, 20, 20);
        }

        Fruit() {
					x = 60;
					y = 60;
					fruitX = x;
					fruitY = y;
        }
				void gotEaten(){
					if (FruitEaten) {	
						//printf("Fruit got eaten");
						FruitEaten = false;
						x = rand()%20 * 40;
						y = rand()%20 * 30;
						if (y % 20 != 0) y = y - 10;
						fruitX = x;
						fruitY = y;
						//printf("Fruit new coordinate: %d, %d", x ,y);
					}
				}

    private:
        int x;
        int y;
};
class Text : public Displayable {
public:
  virtual void paint(XInfo& xinfo) {
    XDrawImageString( xinfo.display, xinfo.window, xinfo.gc[0],
                      this->x, this->y, this->s.c_str(), this->s.length() );
  }

  // constructor
  Text(int x, int y, string s): x(x), y(y), s(s)  {}  

private:
  int x;
  int y;
  string s; // string to show
};

list<Displayable *> dList;           // list of Displayables
list<Displayable *> startList;
list<Displayable *> endList;


Snake snake(100, 460);
Fruit fruit;
Text fps(703, 525, "FPS: " + convertInt(FPS));
Text speed(700, 550, "Speed: " + convertInt(Speed/2));
Text score(45, 575, "Score: " + convertInt(Score));
Text instructions1(200,200, "Eat the fruit to gain more points, avoid hitting the edges or biting yourself.");
Text instructions2(200,220, "Use [w]up, [a]left, [d]right, [s]down to move.");
Text instructions3(200,240, "Press [g] to start the game, [p] to pause and [q] to quit");
Text createdBy(250, 290, "Created by Shashwat Pratap with <3");
Text StudentID(250, 340, "USERID: spratap");
Text TextOnGameEnd(100, 200, "Game Over!! You died :( Press r to restart or q to quit");
Text FinalScore(100, 400, "Your score: " + convertInt(Score));
//startList.push_front(&instructions);
//startList.push_front(&createdBy);
//startList.push_front(&StudentID);
//endList.push_front(&TextOnGameEnd);
//endList.push_front(&FinalScore);


void GameBegin() {
  Up = false;
  Down = false;
  Left = false;
  Right = true; 
  Score = 0;
  snake = Snake(100, 460);
  fruit = Fruit();
  GameStart = true;
  GameOver = false;
  GamePaused = false;
  StartSplashScreen = false;
  
}



/*
 * Initialize X and create a window
 */
void initX(int argc, char *argv[], XInfo &xInfo) {
	XSizeHints hints;
	unsigned long white, black;

   /*
	* Display opening uses the DISPLAY	environment variable.
	* It can go wrong if DISPLAY isn't set, or you don't have permission.
	*/	
	xInfo.display = XOpenDisplay( "" );
	if ( !xInfo.display )	{
		error( "Can't open display." );
	}
	
   /*
	* Find out some things about the display you're using.
	*/
	xInfo.screen = DefaultScreen( xInfo.display );

	white = XWhitePixel( xInfo.display, xInfo.screen );
	black = XBlackPixel( xInfo.display, xInfo.screen );

	hints.x = 100;
	hints.y = 100;
	hints.width = 800;
	hints.height = 600;
	hints.flags = PPosition | PSize;

	xInfo.window = XCreateSimpleWindow( 
		xInfo.display,				// display where window appears
		DefaultRootWindow( xInfo.display ), // window's parent in window tree
		hints.x, hints.y,			// upper left corner location
		hints.width, hints.height,	// size of the window
		Border,						// width of window's border
		black,						// window border colour
		white );					// window background colour
		
	XSetStandardProperties(
		xInfo.display,		// display containing the window
		xInfo.window,		// window whose properties are set
		"Snake",		// window's title
		"Animate",			// icon's title
		None,				// pixmap for the icon
		argv, argc,			// applications command line args
		&hints );			// size hints for the window

	/* 
	 * Create Graphics Contexts
	 */
	int i = 0;
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	XSetForeground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	XSetLineAttributes(xInfo.display, xInfo.gc[i],
	                     1, LineSolid, CapButt, JoinRound);

	XSelectInput(xInfo.display, xInfo.window, KeyPressMask); 

	/*
	 * Put the window on the screen.
	 */
	XMapRaised( xInfo.display, xInfo.window );
	XFlush(xInfo.display);
}

/*
 * Function to repaint a display list
 */
void repaint( XInfo &xinfo) {
	list <Displayable *>::const_iterator begin, end;
	//printf("StartSplashScreen is %d, GameStart is %d, GameOver is %d", StartSplashScreen, GameStart, GameOver);
	if(StartSplashScreen) {
		begin = startList.begin();
    end = startList.end();
	}
	else if(GameOver) {
		FinalScore = Text(100, 400, "Your score: " + convertInt(Score));
		begin = endList.begin();
		end = endList.end();
	}
	else if(GameStart) {
		score = Text(45, 575, "Score: " + convertInt(Score));
		begin = dList.begin();
    end = dList.end();

	}
	XClearWindow( xinfo.display, xinfo.window );
	
	// get height and width of window (might have changed since last repaint)

	//XWindowAttributes windowInfo;
	//XGetWindowAttributes(xinfo.display, xinfo.window, &windowInfo);
	//unsigned int height = windowInfo.height;
	//unsigned int width = windowInfo.width;

	// big black rectangle to clear background
    
	// draw display list
	while( begin != end ) {
		Displayable *d = *begin;
		d->paint(xinfo);
		begin++;
	}
	XFlush( xinfo.display );
}


void handleKeyPress(XInfo &xinfo, XEvent &event) {
	KeySym key;
	char text[BufferSize];

	/*
	 * Exit when 'q' is typed.
	 * This is a simplified approach that does NOT use localization.
	 */
	int i = XLookupString( 
			(XKeyEvent *)&event, 	// the keyboard event
			text, 					// buffer when text will be written
			BufferSize, 			// size of the text buffer
			&key, 					// workstation-independent key symbol
			NULL );					// pointer to a composeStatus structure (unused)
	if ( i == 1) {
		printf("Got key press -- %c\n", text[0]);
		if (text[0] == 'q') {
			error("Terminating normally.");
		}
		else if(text[0] == 'w'){
			if(!Up && !Down) {
				Up = true;
				Down = false;
				Left = false;
				Right = false;

			}
		}
		else if(text[0] == 'a'){
			if(!Left && !Right) {
				Left = true;
				Right = false;
				Up = false;
				Down = false;
			}
		}
		else if(text[0] == 'd'){
			if(!Left && !Right) {
				Left = false;
				Right = true;
				Up = false;
				Down = false;
			}
		}
		else if(text[0] == 's'){
			if(!Up && !Down) {
				Left = false;
				Right = false;
				Up = false;
				Down = true;
			}
		}
		else if(text[0] == 'r'){
			printf("Restart the game\n");
			GameBegin();
		}
		else if(text[0] == 'p'){
			printf("Pause the game\n");
			GamePaused = !GamePaused;
		}
		else if(text[0] == 'g') {
			printf("Start the game\n");
			if (StartSplashScreen) {
				GameBegin();
			}	
		}
	}
}

void handleAnimation(XInfo &xinfo, int inside) {
/*
* This method handles animation for different objects on the screen and readies the next frame before the screen is re-painted.
*/ 
	if (!GamePaused) {
	//printf("Snake Moved\n");
	snake.move(xinfo);
	fruit.gotEaten();
	}
}

// get microseconds
unsigned long now() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

void eventLoop(XInfo &xinfo) {
	// Add stuff to paint to the display list
	dList.push_front(&snake);
  dList.push_front(&fruit);
	dList.push_front(&fps);
	dList.push_front(&score);
	dList.push_front(&speed);	
	startList.push_front(&instructions1);
startList.push_front(&instructions2);
startList.push_front(&instructions3);
 startList.push_front(&createdBy);
 startList.push_front(&StudentID);
 endList.push_front(&TextOnGameEnd);
 endList.push_front(&FinalScore);

int inside = 0;
	int counter = 0;
	int limit = FPS/Speed;
	XEvent event;
	unsigned long lastRepaint = 0;
	unsigned long TimeBefore, TimeAfter, TimeTaken;
	//printf("In splash screen StartSplashScreen is %d, GameStart is %d, GameOver is %d \n", StartSplashScreen, GameStart, GameOver);
	while( true ) {
		TimeBefore = now();	
		if (XPending(xinfo.display) > 0) {
			XNextEvent( xinfo.display, &event );
			//cout << "event.type=" << event.type << "\n";
			switch( event.type ) {
				case KeyPress:
					handleKeyPress(xinfo, event);
					break;
			}
		}
		
		counter = counter + 1;
		if (GameStart == false) {
			counter = 0;
		}
		if (counter == limit && GameStart == true) {
			handleAnimation(xinfo, inside);
			counter = 0;
		}
		repaint(xinfo);
		TimeAfter = now();
		TimeTaken = TimeAfter - TimeBefore;
		//printf("Time Taken to perform Tasks: %ld \n", TimeTaken);
		usleep(1000000/FPS - TimeTaken);
		//TimeAfter = now();
		//TimeTaken = TimeAfter - TimeBefore;
		//printf("Time Taken by Event Loop: %ld \n", TimeTaken);
	}
}


/*
 * Start executing here.
 *	 First initialize window.
 *	 Next loop responding to events.
 *	 Exit forcing window manager to clean up - cheesy, but easy.
 */
int main ( int argc, char *argv[] ) {
	if (argc == 2) {
		stringstream ss(argv[1]);
		ss >> FPS;
		Speed = 5;
	}
	else if (argc == 3) {
		stringstream ss1(argv[1]);
		stringstream ss2(argv[2]);	
		ss1 >> FPS;
		ss2 >> Speed;
		
	}
	else if (argc ==1) {
		FPS = 30;
		Speed = 5;
	}
	else if (argc != 1) {
		error("Wrong number of arguments passed");
	}
	Speed = Speed * 2;
	//printf("FPS: %d, Speed: %d \n", FPS, Speed);
	fps = Text(703, 525, "FPS: " + convertInt(FPS));
	speed = Text(700, 550, "Speed: " + convertInt(Speed/2));
	score = Text(45, 575, "Score: " + convertInt(Score));
	XInfo xInfo;
	initX(argc, argv, xInfo);
	eventLoop(xInfo);
	XCloseDisplay(xInfo.display);
}

