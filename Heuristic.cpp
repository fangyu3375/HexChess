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

int main()
{
#ifndef _BOTZONE_ONLINE
	freopen("in.txt", "r", stdin);
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



	// 向平台输出决策结果
	// cout << bestAction.x << ' ' << bestAction.y << endl;
	return 0;
}