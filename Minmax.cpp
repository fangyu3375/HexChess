#include <iostream>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <deque>

using namespace std;

// typedef pair<int, pair<int, int>> PIP;

const int SIZE = 11, MAX_DEPTH = 10, MAX_BRANCH= 15, INF = 0x3f3f3f3f;

int board[SIZE][SIZE] = { 0 };//本方1，对方-1，空白0
enum Player{Red, Blue};
bool isFirstPlayer;     // true为红方（连接上下边界），false为蓝方（连接左右边界）

struct Position {
    int x, y;
};

struct MinmaxTree {
    int depth;
    int selfboard[SIZE][SIZE];
    Position decision;
    MinmaxTree* parent;
    vector<MinmaxTree*> children;
    // double alpha, beta;
    double score;
    MinmaxTree(int b[][SIZE], double sc, Position pos, MinmaxTree* t) 
    :score(sc), decision(pos), parent(t)
    {memcpy(selfboard, b, sizeof(int) * SIZE * SIZE);}
    bool operator<(const MinmaxTree& t) {return score < t.score;}
};

int bound[2] = {0, SIZE - 1};
int distFrom[4][SIZE][SIZE];        // {0, 1, 2, 3} 分别表示 {上, 左, 下, 右}
int dx[6] = {-1, -1, 0, 1, 1, 0}, dy[6] = {0, 1, 1, 0, -1, -1};  // 方向向量
bool st[SIZE][SIZE];

double evalute(MinmaxTree* node) {
    return node->score;
}

void GenerateSearchSpace(MinmaxTree* node) {
    if (node->depth >= MAX_DEPTH) return;

    expand(node);              // 生成孩子节点，child->depth = node->depth + 1;
    for (auto child : node->children) expand(child);
}

// 估值函数，用来对当前局面的先手优势的估值
// Valuation function1.0: 使用电阻电路策略估值
// 电阻电路策略：当前局面下，先手获胜需要的最小步数 / 后手获胜需要的最小步数
// 可以将先手方落子处的电阻设为0，后手方落子处的电阻设为正无穷，空子处设为1，计算最短路问题
// 电阻正无穷处无法落子，那么相当于边权为0和1最短路问题，可采用双端队列进行求解
double expand(MinmaxTree* node) {
    memset(distFrom, 0x3f, sizeof distFrom);
    memset(st, false, sizeof st); 
    int redValue = isFirstPlayer ? 1 : -1;  // 判断红方是board[][]中值为1的点还是-1的点
    for (int d = 0; d < 4; ++d) {       // 对四个方向进行计算
        int i, j;     // i表示枚举下标，j表示初始边界
        int selfValue = (d & 1)? -redValue : redValue;  // selfValue表示连接d方向的棋子在board[][]中的值
        j = (d & 2)? bound[1] : bound[0];   // 若d为2或3，初始边界为SIZE - 1，否则初始边界为0

        deque<pair<int, int>> q;
        for (i = 0; i < SIZE; ++i) {            // 初始时，边界上的棋子入队
            if (d & 1) swap(i, j);              // 若d为1或3，表示横向枚举，此时i为初始边界，j为枚举下标

            if (node->selfboard[i][j] == selfValue) 
                q.push_front({i, j}), distFrom[d][i][j] = 0, st[i][j] = true;
            else if (node->selfboard[i][j] == 0) 
                q.push_back({i, j}), distFrom[d][i][j] = 1, st[i][j] = true;

            if (d & 1) swap(i, j);              
        }

        while (q.size()) {
            auto t = q.front();
            q.pop_front();

            int pos[2] = {t.first, t.second};
            if (pos[d & 1] == SIZE - 1 - j) continue;// 到达对边界：若d为1或3，应判断t.second是否到达对边界

            for (int i = 0; i < 6; ++i) {
                int a = pos[0] + dx[i], b = pos[1] + dy[i];
                if (a < 0 || a >= SIZE || b < 0 || b >= SIZE || st[a][b]) continue;
                int value = node->selfboard[a][b], distance = distFrom[d][pos[0]][pos[1]];
                if (value == -selfValue) continue;
                if (value == 0)  q.push_back({a, b}), distFrom[d][a][b] = distance + 1; 
                else q.push_front({a, b}), distFrom[d][a][b] = distance;
                st[a][b] = true;
            }
        }
    }

    int cnt = 0;
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            if (!node->selfboard[i][j]) {
                int redScore = distFrom[0][i][j] + distFrom[2][i][j] - 1;
                int blueScore = distFrom[1][i][j] + distFrom[3][i][j] - 1;
                double score = (blueScore == 0)? INF : (double)redScore / blueScore;
                // double ε = 1e-6;
                if (score > 1.) {   //?
                    cnt++;
                    MinmaxTree *child = new MinmaxTree(node->selfboard, score, {i, j}, node);
                    child ->selfboard[i][j] = redValue; //?
                    node->children.push_back(child);
                }
            }
    if (cnt > MAX_BRANCH) {
        sort(node->children.begin(), node->children.end());
        while (cnt > MAX_BRANCH)  {
            delete(node->children.back());
            node->children.pop_back();
        }
    }
    return 0.0;
}

double AlphaBeta(MinmaxTree* node, int depth, double alpha, double beta, Player player) {
    if (depth == 0) return evalute(node);

    if (player == Red) {
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

    MinmaxTree* gameRoot = new MinmaxTree(board, -1, {-1, -1}, nullptr);
    expand(gameRoot);
    if (isFirstPlayer) AlphaBeta(gameRoot, MAX_DEPTH, -INF, INF, Red);
    else AlphaBeta(gameRoot, MAX_DEPTH, -INF, INF, Blue);
    Position bestAction = getOptimalDecision(gameRoot);

	// 向平台输出决策结果
	cout << bestAction.x << ' ' << bestAction.y << endl;
	return 0;
}