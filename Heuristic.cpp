#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <vector>

using namespace std;

const int SIZE = 11, MAX_DEPTH = 10, INF = 0x7ffffff;

int board[SIZE][SIZE] = { 0 };//本方1，对方-1，空白0
enum Player{Red, Blue};
bool isFirstPlayer;     // true为红方（连接上下边界），false为蓝方（连接左右边界）

struct Position {
    int x, y;
};

bool avail(int x, int y) {
    if (x >= 0 && x < SIZE && y >= 0 && y < SIZE && !board[x][y]) return true;
    return false;
}

Position Heuristic(int n, int x, int y) {
	if (!isFirstPlayer && n == 1) return {7, 3};
	
	if (isFirstPlayer)
    {
    	if (x == 1 && y == 8 && board[2][8] == 1 && avail(0, 10) && avail(1, 9) && avail(1, 10))
			return {1, 9};
		
		if (x == 9 && y == 2 && board[8][2] == 1 && avail(9, 0) && avail(9, 1) && avail(10, 0))
			return {9, 1};
		
        if (y == SIZE - 3 && x != 0) {
            int attack = 0;
			attack=attack-board[x][y-1]-board[x+1][y-1]-board[x+1][y-2];					
            if (!board[x][y + 1] && !board[x - 1][y + 1] && !board[x - 1][y + 2] && attack >= 1)
                return {x - 1, y + 2};
        }
        if (y == 2 && x != SIZE - 1) { 
			int attack = 0;	
			attack=attack-board[x][y+1]-board[x-1][y+1]-board[x-1][y+2];		
            if (!board[x][y - 1] && !board[x + 1][y - 1] && !board[x + 1][y - 2]&& attack >= 1)
                return {x + 1, y - 2};
        }

        if (y == SIZE - 4 && x != 0 && x != 1)
        {
            int point = 0;
            point += board[x][y - 1] + board[x + 1][y + 1] + board[x + 1][y - 2];
            if (avail(x - 1, y + 1) && avail(x, y + 1) && avail(x - 1, y + 2) 
            && avail(x - 1, y + 3) && avail(x - 2, y + 3) && point < 0)
                return {x - 1, y + 2};
        }

        if (y == 3 && x != SIZE - 1 && x != SIZE - 2)
        {
            int point = 0;
            point += board[x][y + 1] + board[x - 1][y + 1] + board[x - 1][y + 2];
            if (avail(x + 1, y - 1) && avail(x, y - 1) && avail(x + 1, y - 2) 
            && avail(x + 1, y - 3) && avail(x + 2, y - 3) && point < 0)
                return {x + 1,y - 2};
        }

        //对方拦堵
        if (x == SIZE - 1 && y != SIZE - 1 && board[x - 2][y + 1] == 1)
            if (avail(x, y - 1) && avail(x, y - 2) && avail(x - 1, y) 
            && avail(x - 1, y - 1) && avail(x - 2, y)) 
                return {x - 1, y - 1};
        else if (avail(x, y + 1) && avail(x, y + 2) && avail(x - 1, y + 1)
            && avail(x - 1, y + 2), avail(x - 2, y + 2))
                return {x - 1, y + 2};
        if (x == 0 && y != 0 && board[x + 2][y - 1] == 1)
            if (avail(x, y - 1) && avail(x, y - 2) && avail(x + 1, y - 1) 
            && avail(x + 1, y - 2) && avail(x + 2, y - 2))
                return {x + 1, y - 2};
        else if (avail(x, y + 1) && avail(x, y + 2) && avail(x + 1, y) 
            && avail(x + 1, y + 1) && avail(x + 2, y))
                return {x + 1, y + 1};
    }
    
    else
    {
    	if (x == 2 && y == 9 && board[2][8] == 1 && avail(0, 10) && avail(1, 9) && avail(1, 10))
			return {1, 9};
		
		if (x == 8 && y == 1 && board[8][2] == 1 && avail(9, 0) && avail(9, 1) && avail(10, 0))
			return {9, 1};
        if (x == SIZE - 3 && y != 0){
        	int attack = 0;
			attack=attack-board[x-1][y]-board[x-1][y+1]-board[x-2][y+1];	
            if (!board[x + 1][y] && !board[x + 1][y - 1] && !board[x + 2][y - 1] && attack >= 1)
                return {x + 2, y - 1};
        }
        if (x == 2 && y != SIZE - 1){
		    int attack = 0;
			attack=attack-board[x+1][y-1]-board[x+1][y]-board[x+2][y-1];
            if (!board[x - 1][y] && !board[x - 1][y + 1] && !board[x - 2][y + 1] && attack >= 1)
                return {x - 2, y + 1};
        }
        if (x == SIZE - 4 && y != 0 && y != -1)
        {
            int point = 0;
            point += board[x - 1][y] + board[x - 1][y + 1] + board[x - 2][y + 1];
            if (avail(x + 1, y - 1) && avail(x + 1, y) && avail(x + 2, y - 1)
            && avail(x + 3, y - 1) && avail(x + 3, y - 2) && point < 0)
                return {x + 2, y - 1};
        }

        if (x == 3 && y != SIZE - 1 && y != SIZE - 2)
        {
            int point = 0;
            point += board[x + 1][y] + board[x + 1][y - 1] + board[x + 2][y - 1];
            if (avail(x - 1, y + 1) && avail(x - 1, y) && avail(x - 2, y + 1)
            && avail(x - 3, y + 1) && avail(x - 3, y + 2) && point < 0)
                return {x - 2, y + 1};
        }

        //对方拦堵
        if (y == SIZE - 1 && x != SIZE - 1 && board[x + 1][y - 2] == 1)
            if (avail(x, y - 1) && avail(x, y - 2) && avail(x - 1, y) 
            && avail(x - 1, y - 1) && avail(x - 2, y))
                return {x - 1, y - 1};
        else if (avail(x + 1, y) && avail(x + 1, y - 1) && avail(x + 2, y)
            && avail(x + 2, y - 1), avail(x + 2, y - 2))
                return {x + 2, y - 1};
        if (y == 0 && x != 0 && board[x - 1][y + 2] == 1)
            if (avail(x - 1, y) && avail(x - 2, y) && avail(x - 1, y + 1) 
            && avail(x - 2, y + 1) && avail(x + 3, y + 2))
                return {x - 2, y + 1};
        else if (avail(x, y + 1) && avail(x, y + 2) && avail(x + 1, y) 
            && avail(x + 1, y + 1) && avail(x + 2, y))
                return {x + 1, y + 1};
    }
	
}

int main()
{
#ifndef _BOTZONE_ONLINE
	freopen("in.txt", "r", stdin);
	freopen("out.txt", "w", stdout);
#endif // !_BOTZONE_ONLINE

	int x, y, n;
	//恢复目前的棋盘信息
	cin >> n;
	for (int i = 0; i < n - 1; i++){
		cin >> x >> y; if (x != -1) board[x][y] = -1;	//对方
		cin >> x >> y; if (x != -1) board[x][y] = 1;	//我方
	}
	cin >> x >> y; 
	if (x != -1) board[x][y] = -1;	//对方
	else {
		cout << 1 << ' ' << 2 << endl;
		return 0;
	}
	//此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋

    if(board[1][2] == 1) isFirstPlayer= true; // 我方为红方

	//启发式规则
	Position bestAction = Heuristic(n, x, y);

	// 向平台输出决策结果
	cout << bestAction.x << ' ' << bestAction.y << endl;
	return 0;
}