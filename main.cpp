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
		Up.Clear();
		Down.Clear();
		Left.Clear();
		Right.Clear();
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
int UID = 0;

static int atoi_s(const char *arg, int def) {
    int val = atoi(arg);
    if (val==0 && errno==EINVAL) {
        return def;
    }
    return val;
}

int main(int argc, char **argv)
{
	/*
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t1);
	QueryPerformanceCounter(&t2);
	double deltat = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	*/
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
	return 0;
}
