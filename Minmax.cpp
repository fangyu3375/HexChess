#include <iostream>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <deque>

using namespace std;

const int SIZE = 11, MAX_DEPTH = 5, MAX_BRANCH= 15, INF = 0x3f3f3f3f;
const double C = 0.8;

int board[SIZE][SIZE] = { 0 };//本方1，对方-1，空白0
enum Player{Red, Blue};
bool isFirstPlayer;     // true为红方（连接上下边界），false为蓝方（连接左右边界）
int redValue;

struct Position {
    int x, y;
};

struct MinmaxTree {
    int depth;
    double score;
    Position decision;
    MinmaxTree* parent;
    vector<MinmaxTree*> children;
    MinmaxTree(MinmaxTree* t = nullptr, int d = 0, Position pos = {-1, -1}, double sc = 1.) 
    :parent(t), depth(d), decision(pos), score(sc) {}
    ~MinmaxTree() {for (auto child : children) delete(child);}
    bool operator<(const MinmaxTree& t) {return score < t.score;}
    void move(){board[decision.x][decision.y] = (depth & 1)? 1: -1;}  // 奇数层为我方走子，偶数层为对方走子
    void revoke(){board[decision.x][decision.y] = 0;}
};

int dx[6] = {-1, -1, 0, 1, 1, 0}, dy[6] = {0, 1, 1, 0, -1, -1};  // 方向向量

// 估值函数，用来对当前局面的先手优势的估值，棋局保存在全局变量board[][]中
// Valuation function1.0: 使用电阻电路策略估值
// 电阻电路策略：当前局面下，先手获胜需要的最小步数 / 后手获胜需要的最小步数
// 可以将先手方落子处的电阻设为0，后手方落子处的电阻设为正无穷，空子处设为1，计算最短路问题
// 电阻正无穷处无法落子，那么相当于边权为0和1最短路问题，可采用双端队列进行求解
double evalute() {
    int dist[SIZE][SIZE], minDist[2];
    for (int d = 0; d < 2; ++d) {       // 对上下和左右两个方向进行计算, d == 0:上下, d == 1:左右
        memset(dist, -1, sizeof dist);
        int selfValue = (d & 1)? -redValue : redValue;  // selfValue表示连接d方向的棋子在board[][]中的值

        deque<pair<int, int>> q;
        for (int i = 0, j = 0; j < SIZE; ++j) {  // i表示枚举下标，j表示初始边界 
            if (d & 1) swap(i, j);              // 若d为1，表示横向枚举，此时i为初始边界，j为枚举下标

            // 初始时，边界上的点入队
            if (board[i][j] == selfValue) 
                q.push_front({i, j}), dist[i][j] = 0;
            else if (board[i][j] == 0) 
                q.push_back({i, j}), dist[i][j] = 1;

            if (d & 1) swap(i, j);              
        }

        while (q.size()) {
            auto t = q.front();
            q.pop_front();

            int pos[2] = {t.first, t.second};
            int distance = dist[pos[0]][pos[1]];
            if (pos[d & 1] == SIZE - 1) {   // 到达对边界：若d为1，应判断t.second是否到达对边界
                minDist[d & 1] = distance;
                break;
            }

            for (int i = 0; i < 6; ++i) {
                int a = pos[0] + dx[i], b = pos[1] + dy[i];
                if (a < 0 || a >= SIZE || b < 0 || b >= SIZE || ~dist[a][b]) continue;
                if (board[a][b] == -selfValue) continue;

                if (board[a][b] == 0)  q.push_back({a, b}), dist[a][b] = distance + 1; 
                else q.push_front({a, b}), dist[a][b] = distance;
            }
        }
    }

    // cout << minDist[0] << ' ' << minDist[1] << endl;
    if (minDist[0] == 0) return INF;
    return C * minDist[1] - (1 - C) * minDist[0];
}

// 朴素版扩展
// 只限定扩展层数，而不要求子节点是否对己方有利
void expand(MinmaxTree* node) {
    if (node->depth >= MAX_DEPTH) return;
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            if (board[i][j] == 0) {
                MinmaxTree* child = new MinmaxTree(node, node->depth + 1, {i, j});
                node->children.push_back(child);
                child->move(), expand(child), child->revoke();
            }
                
}

double AlphaBeta(MinmaxTree* node, int depth, double alpha, double beta, Player player) {
    if (depth == 0) return evalute();

    if (player == Red) {
        for (auto &child : node->children) {
            child->move();                     // 走子
            alpha = max(alpha, AlphaBeta(child, depth - 1, alpha, beta, Blue));
            if (alpha >= beta) break;          // alpha剪枝 
            child->revoke();                   // 回溯
        }
        return alpha;
    } else {
        for (auto &child : node->children) {
            child->move();
            beta = min(beta, AlphaBeta(child, depth - 1, alpha, beta, Red));
            child->revoke();   
            if (beta <= alpha) break;
        }
        return beta;
    }
}

Position getOptimalDecision(MinmaxTree* t) {
    MinmaxTree* res;
    if (isFirstPlayer) res = *max_element(t->children.begin(), t->children.end());
    else res = *min_element(t->children.begin(), t->children.end());
    return res->decision;
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
    redValue = isFirstPlayer ? 1 : -1;  // 判断红方是board[][]中值为1的点还是-1的点

    MinmaxTree* gameRoot = new MinmaxTree();
    expand(gameRoot);
    if (isFirstPlayer) AlphaBeta(gameRoot, MAX_DEPTH, -INF, INF, Red);
    else AlphaBeta(gameRoot, MAX_DEPTH, -INF, INF, Blue);
    Position bestAction = getOptimalDecision(gameRoot);

	// 向平台输出决策结果
	cout << bestAction.x << ' ' << bestAction.y << endl;
    delete(gameRoot);
	return 0;
}