#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "mm.h"

/************************************************************
*
*	This is a file that will eventually contain various tests
*	and examples for the memory manager.  For now, it
*	contains an extremely hacky implementation of A* 
*	pathfinding.
*
************************************************************/





/*
#include <Windows.h>
LARGE_INTEGER frequency;        // ticks per second
LARGE_INTEGER t1, t2;           // ticks
double elapsedTime;
*/

class Node {
public:
	Pointer<Node> Up;
	Pointer<Node> Down;
	Pointer<Node> Left;
	Pointer<Node> Right;
	bool path;
	bool passable;
	int UID;
	Node() {
		Init();
	}
	~Node() {
		Destroy();
	}
	void Destroy(){
		if (Up && Up.Get().Down) {
			Up.Get().Down.Destroy();
		}
		if (Up)
			Up.Destroy();
		if (Down && Down.Get().Up) {
			Down.Get().Up.Destroy();
		}
		if (Down)
			Down.Destroy();
		if (Left && Left.Get().Right) {
			Left.Get().Right.Destroy();
		}
		if (Left)
			Left.Destroy();
		if (Right && Right.Get().Left) {
			Right.Get().Left.Destroy();
		}
		if (Right)
			Right.Destroy();
	}
	void Init() {
		Up.Init();
		Down.Init();
		Left.Init();
		Right.Init();
		path = false;
		passable = true;
	}
	int RefCount() {
		int out = 0;
		if (Up && Up.Get().Down)
			out++;
		if (Down && Down.Get().Up)
			out++;
		if (Left && Left.Get().Right)
			out++;
		if (Right && Right.Get().Left)
			out++;
		return out;
	}
};

mm* Mem;

static int atoi_s(const char *arg, int def) {
    int val = atoi(arg);
    if (val==0 && errno==EINVAL) {
        return def;
    }
    return val;
}


#define MAX_X 45
#define MAX_Y 22

float EstimateCost(int start, int end) {
	Pointer<int> x1;
	x1.Allocate();
	*x1 = start % MAX_X;
	Pointer<int> y1;
	y1.Allocate();
	*y1 = start / MAX_X;
	Pointer<int> x2;
	x2.Allocate();
	*x2 = end % MAX_X;
	Pointer<int> y2;
	y2.Allocate();
	*y2 = end / MAX_X;
	return float(abs(*x2 - *x1) + abs(*y2 - *y1));
}

int main(int argc, char **argv)
{
	/*
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t1);
	QueryPerformanceCounter(&t2);
	double deltat = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	*/
	//*
	Pointer<Node> Empty;
	Pointer<int> mx, my, sx, sy, gx, gy;
	Pointer<int> UID;
	UID.Allocate();
	*UID = 0;
	Empty.Init();
	mx.Allocate();
	*mx = MAX_X;
	my.Allocate();
	*my = MAX_Y;
	sx.Allocate();
	*sx = 1;
	sy.Allocate();
	*sy = 1;
	gx.Allocate();
	*gx = MAX_X - 2;
	gy.Allocate();
	*gy = MAX_Y - 2;

	if (argc>1) {
		//*mx = atoi_s(argv[1], *mx);
	}
	if (argc>2) {
		//*my = atoi_s(argv[2], *my);
	}
	if (argc>3) {
		*gx = atoi_s(argv[3], *gx);
	}
	else if (*gx>*mx - 2) {
		*gx = *mx - 2;
	}
	if (argc>4) {
		*gy = atoi_s(argv[4], *gy);
	}
	else if (*gy>*my - 2) {
		*gy = *my - 2;
	}
	//assert(*mx>0 && *my>0 && *sx <= *mx && *sy <= *my && *gx <= *mx && *gy <= *my);
	printf("mx=%d, my=%d, gx=%d, gy=%d\n", *mx, *my, *gx, *gy);
	Mem = (mm*)&(mm::get());

	Pointer<Pointer<Node>, 1> Grid;
	Grid.Allocate();
	Grid.Resize(MAX_X*MAX_Y);
	Pointer<int> x, y;
	x.Allocate();
	y.Allocate();
	for (*y = 0; *y < *my; ++*y) {
		for (*x = 0; *x < *mx; ++*x) {
			Grid[*x + (*y**mx)].Allocate();
			Grid[*x + (*y**mx)].Get().UID = *UID;
			*UID = *UID + 1;
		}
	}
	for (*y = 0; *y < *my; ++*y) {
		for (*x = 0; *x < *mx; ++*x) {
			if ((*x == 0) || (*y == 0) || (*x >= *mx - 1) || (*y >= *my - 1)){
				Grid[*x + (*y**mx)].Get().passable = false;
			}
			if (*x > 0) {
				Grid[*x + (*y**mx)].Get().Left = Grid[(*x - 1) + ((*y)**mx)];
			}
			if (*x < *mx - 1) {
				Grid[*x + (*y**mx)].Get().Right = Grid[(*x + 1) + ((*y)**mx)];
			}
			if (*y > 0) {
				Grid[*x + (*y**mx)].Get().Up = Grid[(*x) + ((*y - 1)**mx)];
			}
			if (*y < *my - 1) {
				Grid[*x + (*y**mx)].Get().Down = Grid[(*x) + ((*y + 1)**mx)];
			}
		}
	}

	Pointer<Node> walker;
	for (*x = 0; *x < *mx/10; ++*x) {
		walker = Grid[*x * 10 + 5 + 3 * *mx];
		while (walker) {
			(*walker).passable = false;
			walker = (*walker).Down;
		}
		walker = Grid[*x * 10 + 10 + (*my - 3) * *mx];
		while (walker) {
			(*walker).passable = false;
			walker = (*walker).Up;
		}
	}
	// A* pathfinding


	Pointer<Pointer<Node>[MAX_X*MAX_Y]> ClosedSet;
	Pointer<Pointer<Node>[MAX_X*MAX_Y]> OpenSet;
	Pointer<Pointer<Node>[MAX_X*MAX_Y]> CameFrom;
	Pointer<float[MAX_X*MAX_Y]> gScore;
	Pointer<float[MAX_X*MAX_Y]> fScore;
	Pointer<float> TentScore;
	TentScore.Allocate();
	ClosedSet.Init();
	ClosedSet.Allocate();
	OpenSet.Init();
	OpenSet.Allocate();
	CameFrom.Init();
	CameFrom.Allocate();
	gScore.Init();
	gScore.Allocate();
	fScore.Init();
	fScore.Allocate();
	Pointer<bool> NotEmpty;
	NotEmpty.Allocate();
	*NotEmpty = true;
	
	(*OpenSet)[Grid[*sx + *sy*MAX_X].Get().UID] = Grid[*sx + *sy*MAX_X];

	Pointer<Node> Current;
	Pointer<Node> Goal;
	Goal = Grid[*gx + *gy*MAX_X];
	(*fScore)[Grid[*sx + *sy*MAX_X].Get().UID] = EstimateCost(Grid[*sx + *sy*MAX_X].Get().UID, Goal.Get().UID);
	while (*NotEmpty) {
		*NotEmpty = false;
		for (*x = 0; *x < MAX_X*MAX_Y; ++*x) {
			if (!Current) {
				if ((*OpenSet)[*x]) {
					Current = (*OpenSet)[*x];
					*NotEmpty = true;
				}
			}
			else {
				if ((*OpenSet)[*x]) {
					if ((*fScore)[Current.Get().UID] > (*fScore)[(*OpenSet)[*x].Get().UID]) {
						Current = (*OpenSet)[*x];
						*NotEmpty = true;
					}
				}
			}
		}
		if (!Current)
			break;
		if (Current == Goal)
			break;

		(*OpenSet)[Current.Get().UID] = Empty;
		(*ClosedSet)[Current.Get().UID] = Current;

		if (Current.Get().Up && Current.Get().Up.Get().passable && !(*ClosedSet)[Current.Get().Up.Get().UID]) {
			*TentScore = (*gScore)[Current.Get().UID] + 1.0f;
			if (!(*OpenSet)[Current.Get().Up.Get().UID] || (*TentScore < (*gScore)[Current.Get().Up.Get().UID])) {
				(*CameFrom)[Current.Get().Up.Get().UID] = Current;
				(*gScore)[Current.Get().Up.Get().UID] = *TentScore;
				(*fScore)[Current.Get().Up.Get().UID] = (*gScore)[Current.Get().Up.Get().UID] + EstimateCost(Current.Get().Up.Get().UID, Goal.Get().UID);
				(*OpenSet)[Current.Get().Up.Get().UID] = Current.Get().Up;
			}
		}

		if (Current.Get().Down && Current.Get().Down.Get().passable && !(*ClosedSet)[Current.Get().Down.Get().UID]) {
			*TentScore = (*gScore)[Current.Get().UID] + 1.0f;
			if (!(*OpenSet)[Current.Get().Down.Get().UID] || (*TentScore < (*gScore)[Current.Get().Down.Get().UID])) {
				(*CameFrom)[Current.Get().Down.Get().UID] = Current;
				(*gScore)[Current.Get().Down.Get().UID] = *TentScore;
				(*fScore)[Current.Get().Down.Get().UID] = (*gScore)[Current.Get().Down.Get().UID] + EstimateCost(Current.Get().Up.Get().UID, Goal.Get().UID);
				(*OpenSet)[Current.Get().Down.Get().UID] = Current.Get().Down;
			}
		}

		if (Current.Get().Left && Current.Get().Left.Get().passable && !(*ClosedSet)[Current.Get().Left.Get().UID]) {
			*TentScore = (*gScore)[Current.Get().UID] + 1.0f;
			if (!(*OpenSet)[Current.Get().Left.Get().UID] || (*TentScore < (*gScore)[Current.Get().Left.Get().UID])) {
				(*CameFrom)[Current.Get().Left.Get().UID] = Current;
				(*gScore)[Current.Get().Left.Get().UID] = *TentScore;
				(*fScore)[Current.Get().Left.Get().UID] = (*gScore)[Current.Get().Left.Get().UID] + EstimateCost(Current.Get().Left.Get().UID, Goal.Get().UID);
				(*OpenSet)[Current.Get().Left.Get().UID] = Current.Get().Left;
			}
		}

		if (Current.Get().Right && Current.Get().Right.Get().passable && !(*ClosedSet)[Current.Get().Right.Get().UID]) {
			*TentScore = (*gScore)[Current.Get().UID] + 1.0f;
			if (!(*OpenSet)[Current.Get().Right.Get().UID] || (*TentScore < (*gScore)[Current.Get().Right.Get().UID])) {
				(*CameFrom)[Current.Get().Right.Get().UID] = Current;
				(*gScore)[Current.Get().Right.Get().UID] = *TentScore;
				(*fScore)[Current.Get().Right.Get().UID] = (*gScore)[Current.Get().Right.Get().UID] + EstimateCost(Current.Get().Right.Get().UID, Goal.Get().UID);
				(*OpenSet)[Current.Get().Right.Get().UID] = Current.Get().Right;
			}
		}
		Current = Empty;
	}

	Current = Goal;
	while (Current) {
		Grid[Current.Get().UID].Get().path = true;
		Current = (*CameFrom)[Current.Get().UID];
	}

	//print the grid
	for (*y = 0; *y < *my; ++*y) {
		for (*x = 0; *x < *mx; ++*x) {
			if (Grid[*x + (*y**mx)].Get().path)
				printf("O");
			else if (Grid[*x + (*y**mx)].Get().passable)
				printf(".");
			else
				printf("X");
		}
		printf("\n");
	}
	return 0;
}
