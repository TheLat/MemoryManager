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
*	contains an extremely hacky implementation of Djikstra's
*	algorith.
*
*	Testing with A* now.  Hopefully less hacky this time around.
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
		Pointer<Node> Empty;
		Up = Empty;
		Down = Empty;
		Left = Empty;
		Right = Empty;
	}
	void Init() {
		Up.Init();
		Down.Init();
		Left.Init();
		Right.Init();
		path = false;
		passable = true;
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
	int sx = start % MAX_X;
	int sy = start / MAX_X;
	int gx = end % MAX_X;
	int gy = end / MAX_X;
	return float(abs(gx - sx) + abs(gy - sy));
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

	Pointer<Pointer<Node>[MAX_X*MAX_Y]> Grid;
	Grid.Init();
	Grid.Allocate();
	Pointer<int> x, y;
	x.Allocate();
	y.Allocate();
	for (*y = 0; *y < *my; ++*y) {
		for (*x = 0; *x < *mx; ++*x) {
			(*Grid)[*x + (*y**mx)].Allocate();
			(*Grid)[*x + (*y**mx)].Get().UID = *UID;
			*UID = *UID + 1;
		}
	}

	for (*y = 0; *y < *my; ++*y) {
		for (*x = 0; *x < *mx; ++*x) {
			if ((*x == 0) || (*y == 0) || (*x >= *mx - 1) || (*y >= *my - 1)){
				(*Grid)[*x + (*y**mx)].Get().passable = false;
			}
			if (*x > 0) {
				(*Grid)[*x + (*y**mx)].Get().Left = (*Grid)[(*x - 1) + ((*y)**mx)];
			}
			if (*x < *mx - 1) {
				(*Grid)[*x + (*y**mx)].Get().Right = (*Grid)[(*x + 1) + ((*y)**mx)];
			}
			if (*y > 0) {
				(*Grid)[*x + (*y**mx)].Get().Up = (*Grid)[(*x) + ((*y - 1)**mx)];
			}
			if (*y < *my - 1) {
				(*Grid)[*x + (*y**mx)].Get().Down = (*Grid)[(*x) + ((*y + 1)**mx)];
			}
		}
	}

	Pointer<Node> walker;
	for (*x = 0; *x < *mx/10; ++*x) {
		walker = (*Grid)[*x * 10 + 5 + 3 * *mx];
		while (walker) {
			(*walker).passable = false;
			walker = (*walker).Down;
		}
		walker = (*Grid)[*x * 10 + 10 + (*my - 3) * *mx];
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
	
	(*OpenSet)[(*Grid)[*sx + *sy*MAX_X].Get().UID] = (*Grid)[*sx + *sy*MAX_X];

	Pointer<Node> Current;
	Pointer<Node> Goal;
	Goal = (*Grid)[*gx + *gy*MAX_X];
	(*fScore)[(*Grid)[*sx + *sy*MAX_X].Get().UID] = EstimateCost((*Grid)[*sx + *sy*MAX_X].Get().UID, Goal.Get().UID);
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
		(*Grid)[Current.Get().UID].Get().path = true;
		Current = (*CameFrom)[Current.Get().UID];
	}

	//print the grid
	for (*y = 0; *y < *my; ++*y) {
		for (*x = 0; *x < *mx; ++*x) {
			if ((*Grid)[*x + (*y**mx)].Get().path)
				printf("O");
			else if ((*Grid)[*x + (*y**mx)].Get().passable)
				printf(".");
			else
				printf("X");
		}
		printf("\n");
	}
	//*/

	// Old demo code
	/*
	Pointer<Node> Empty;
	int m = 10;
	int n = 10;
	int sx = 1;
	int sy = 1;
	int gx = 8;
	int gy = 8;
    
    if (argc>1) {
        m = atoi_s(argv[1], m);
    }
    if (argc>2) {
        n = atoi_s(argv[2], n);
    }
    if (argc>3) {
        gx = atoi_s(argv[3], gx);
    } else if (gx>m) {
        gx = m - 2;
    }
    if (argc>4) {
        gy = atoi_s(argv[4], gy);
    } else if (gy>m) {
        gy = n - 2;
    }
    assert(m>0 && n>0 && sx<=m && sy<=n && gx<=m && gy<=n);
    printf("m=%d, n=%d, gx=%d, gy=%d\n", m, n, gx, gy);
	Mem = (mm*)&(mm::get());
	Pointer<Node> *test;
	test = new Pointer<Node>[n*m];
	Pointer<Node> walker;
	walker.Init();
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < m; ++j) {
			test[i + j*n].Init();
			test[i + j*n].Allocate();
		}
	}

	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			(*(test[j + i*n])).passable = true;
			(*(test[j + i*n])).UID = UID;
			UID++;
			if ((i == 0) || (j == 0) || (i == m - 1) || (j == n - 1)) {
				(*(test[j + i*n])).passable = false;
			}
			if (i > 0) {
				(*(test[j + i*n])).Up = ((test[j + (i - 1)*n]));
			}
			if( j > 0) {
				(*(test[j + i*n])).Left = ((test[(j - 1) + i*n]));
            }
			if (j < n - 1) {
				(*(test[j + i*n])).Right = ((test[(j + 1) + i*n]));
			}
			if (i < m - 1) {
				(*(test[j + i*n])).Down = ((test[j + (i + 1)*n]));
            }

		}
	}
	for (int i = 0; i < n / 10; ++i){
		walker = test[(i*10) + 5 + 3 * n];
		while (walker.Get().passable){
			walker.Get().passable = false;
			walker = walker.Get().Down;
		}
		walker = test[(i * 10) + 10 + (m - 3) * n];
		while (walker.Get().passable){
			walker.Get().passable = false;
			walker = walker.Get().Up;
		}
	}
	

	//implement Djikstra's algorithm
	float* dist = new float[UID];
	for (int i = 0; i < UID; ++i) {
		dist[i] = 9999.0f;
	}
	walker = test[sx + sy * n];
	dist[walker.Get().UID] = 0;

	Pointer<Node> *Unvisited = new Pointer<Node>[UID];
	Pointer<Node> *prev = new Pointer<Node>[UID];
	Empty.Init();
	for (int i = 0; i < UID; ++i) {
		Unvisited[i].Init();
		prev[i].Init();
	}
	for (int i = 0; i < n*m; ++i) {
		Unvisited[test[i].Get().UID] = test[i];
		if (!(Unvisited[test[i].Get().UID].Get().passable)) {
			Unvisited[test[i].Get().UID] = Empty;
		}
	}
	Pointer<Node> holder;
	while (walker.IsGood()){
		Unvisited[walker.Get().UID] = Empty;
		float alt;
		alt = dist[walker.Get().UID] + 1.0f;

		holder = walker.Get().Up;
		if (holder.IsGood()) {
			if (holder.Get().passable) {
				if (alt < dist[holder.Get().UID]) {
					prev[holder.Get().UID] = walker;
					dist[holder.Get().UID] = alt;
				}
			}
		}
		holder = walker.Get().Down;
		if (holder.IsGood()) {
			if (holder.Get().passable) {
				if (alt < dist[holder.Get().UID]) {
					prev[holder.Get().UID] = walker;
					dist[holder.Get().UID] = alt;
				}
			}
		}
		holder = walker.Get().Left;
		if (holder.IsGood()) {
			if (holder.Get().passable) {
				if (alt < dist[holder.Get().UID]) {
					prev[holder.Get().UID] = walker;
					dist[holder.Get().UID] = alt;
				}
			}
		}
		holder = walker.Get().Right;
		if (holder.IsGood()) {
			if (holder.Get().passable) {
				if (alt < dist[holder.Get().UID]) {
					prev[holder.Get().UID] = walker;
					dist[holder.Get().UID] = alt;
				}
			}
		}

		walker = Empty;
		for (int i = 0; i < n*m; ++i) {
			if (Unvisited[i].IsGood()) {
				if (walker.IsGood()) {
					if (dist[Unvisited[i].Get().UID] < dist[walker.Get().UID]) {
						walker = Unvisited[i];
					}
				}
				else {
					walker = Unvisited[i];
				}
			}
		}
	}
	int ID = gx + gy*n;
	while (ID != sx + sy*n) {
		prev[ID].Get().path = true;
		ID = prev[ID].Get().UID;
	}







	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			if ((*(test[j + i*n])).path) {
				printf("O");
			}
			else if ((*(test[j + i*n])).passable) {
				printf(".");
			}
			else
				printf("X");
		}
		printf("\n");
	}

	delete[] Unvisited;
	delete[] prev;
	delete[] dist;
	holder.Clear();
	walker.Clear();
	delete [] test;
	//*/
	return 0;
}
