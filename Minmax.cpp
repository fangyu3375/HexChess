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

struct MinmaxTree {
    int selfboard[SIZE][SIZE];
    Position decision;
    MinmaxTree* parent;
    vector<MinmaxTree*> children;
    double alpha, beta;
    MinmaxTree(int b[][SIZE], Position pos, MinmaxTree* t) : decision(pos), parent(t)
    {memcpy(selfboard, b, sizeof(int) * SIZE * SIZE);}
};

double evalute(MinmaxTree* node) {

}

double AlphaBeta(MinmaxTree* node, int depth, double alpha, double beta, Player player) {
    if (depth == 0) return evalute(node);

    if (player = Red) {
        for (auto &child : node->children) {
            alpha = max(alpha, AlphaBeta(child, depth - 1, alpha, beta, Blue));
            if (alpha >= beta) break;          // alpha剪枝 
        }
        return alpha;
    } else {
        for (auto &child : node->children) {
            beta = min(beta, AlphaBeta(child, depth - 1, alpha, beta, Red));
            if (beta <= alpha) break;
        }
        return beta;
    }
}

Position getOptimalDecision(MinmaxTree* t) {
    return {-1, -1};
}

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

    MinmaxTree* gameRoot = new MinmaxTree(board, {-1, -1}, nullptr);
    AlphaBeta(gameRoot, MAX_DEPTH, -INF, INF, Red);
    Position bestAction = getOptimalDecision(gameRoot);

	// 向平台输出决策结果
	cout << bestAction.x << ' ' << bestAction.y << endl;
	return 0;
}