#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <limits.h>
#include <math.h>
#include <chrono>
#include <string.h>
#include <queue>

using namespace std;

const int SIZE = 11, CNT = 60, MAX_DEPTH = 5, MAX_BRANCH= 15, INF = 0x3f3f3f3f;;
const double C = 0.6, TIMELIMIT = 0.96, C2 = 0.8;
const int start=clock(), timeout = (int)(TIMELIMIT*(double)CLOCKS_PER_SEC);

bool isFirstPlayer;     // true为红方（连接上下边界），false为蓝方（连接左右边界）
int board[SIZE][SIZE];  // 我方 1，对方 -1，无子0
int dx[6] = {-1, -1, 0, 1, 1, 0}, dy[6] = {0, 1, 1, 0, -1, -1};  // 方向向量
bool st[SIZE][SIZE];
int number;
int N;

enum Player{Red, Blue};
int redValue;

struct Position {
    int x, y;
};

class MCTtree {
public:      // API
    MCTtree(int[][SIZE], Position, MCTtree*);
    MCTtree* SelectMaxUCBNode();
    void Expand();
    void rollout();
    Position getOptimalDecision();

public:     // getAttributes
    vector<MCTtree*>& getChild();
    double& getNi();

private:    // helper function
    void Backpropagation(int);
    int get_available_pos(int avail_x[], int avail_y[]);
    void cloneBoardTo(int [][SIZE]);
    void CalculateUCB();
    bool Judgement(int board[][SIZE]);

private:    // Attributes
    MCTtree* parent;
    vector<MCTtree*> children;
    Position decision;
    int selfboard[SIZE][SIZE];  //每个节点存一个棋盘
    double value;  //ucb v
    double ni;     //ucb ni;
    double ucb;
};

vector<MCTtree*>& MCTtree::getChild() {
    return this->children;
}

double& MCTtree::getNi() {
    return this->ni;
}

MCTtree::MCTtree(int b[][SIZE], Position pos, MCTtree* p)
    :parent(p), value(0), ni(0), ucb(INT_MAX), decision(pos) {
    memcpy(selfboard, b, sizeof(int) * SIZE * SIZE);
}

// 扩展（模拟走子）
// 在该棋盘状态下随机走子，填满棋盘后判断胜负，反向更新参数
void MCTtree::rollout()
{  
    int copy[SIZE][SIZE];
    cloneBoardTo(copy);

    int avail_x[SIZE * SIZE], avail_y[SIZE * SIZE];
    int cnt = get_available_pos(avail_x, avail_y);

    int turn = -1;
    while(cnt){
        int rand_pos = rand() % cnt;
        int select_x = avail_x[rand_pos];
        int select_y = avail_y[rand_pos];

        copy[select_x][select_y] = turn;
        turn = -turn;

        swap(avail_x[cnt - 1], avail_x[rand_pos]);
        swap(avail_y[cnt - 1], avail_y[rand_pos]);

        cnt--;
    }

    bool redWin = Judgement(copy);
    int score = (redWin ^ isFirstPlayer) ? 0 : 1;   // redWin和isFirstPlayer相同表示我方胜出

    Backpropagation(score);
}

// 将结果反向传播，更新参数
void MCTtree::Backpropagation(int v)
{  
    N++;
    MCTtree* node=this;
    while(node){
        node->value+=v;
        node->ni++;
        node=node->parent;
    }
}

// 判断先手（红方）是否获胜
bool MCTtree::Judgement(int board[][SIZE])
{
    int redValue = isFirstPlayer ? 1 : -1;   // 判断红方是board[][]中值为1的点还是-1的点

    memset(st, false, sizeof st);
    queue<Position> q; 
    for (int i = 0; i < SIZE; ++i)
        if (board[0][i] == redValue)
            q.push({0, i}), st[0][i] = true;
    
    while (!q.empty()) {
        auto t = q.front(); q.pop();
        int x = t.x, y = t.y;
        if (x == SIZE - 1) return true;
        for (int i = 0; i < 6; ++i) {
            int a = x + dx[i], b = y + dy[i];
            if (a < 0 || a >= SIZE || b < 0 || b >= SIZE || st[a][b]) continue;
            if (board[a][b] != redValue) continue;
            q.push({a, b});
            st[a][b] = true;
        }
    }

    return false;
}



void MCTtree::Expand()
{
    int avail_x[SIZE * SIZE], avail_y[SIZE * SIZE];
    int cnt = get_available_pos(avail_x, avail_y);

    for(int i = 0; i < cnt; i++){
        int x = avail_x[i], y = avail_y[i];
        MCTtree* c = new MCTtree(selfboard, {x, y}, this);
        c->selfboard[x][y] = 1;
        children.push_back(c);
    }
    int k = rand()%cnt;
    children[k]->rollout();
}

// 返回 UCB 最大的孩子节点
MCTtree* MCTtree::SelectMaxUCBNode()
{
    for (auto &child : this->children)
        if (child->getNi() != 0)
            child->CalculateUCB();

    MCTtree* maxUCBChild = children[0];
    for (auto &child : this->children)
        if (child->ucb > maxUCBChild->ucb)
            maxUCBChild = child;
    
    return maxUCBChild;
}

// 得到当前棋局可走的合法位置表， 并返回个数
int MCTtree::get_available_pos(int avail_x[], int avail_y[])
{
    int cnt = 0;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (selfboard[i][j] == 0) {
                avail_x[cnt] = i;
                avail_y[cnt] = j;
                cnt++;
            }
    return cnt;
}	

void MCTtree::cloneBoardTo(int copy[][SIZE]) {
    memcpy(copy, selfboard, sizeof(int) * SIZE * SIZE);
}

void MCTtree::CalculateUCB() {
    this->ucb = value / ni + C * sqrt(log(N) / ni);
}


Position MCTtree::getOptimalDecision()
{
    MCTtree* optimalChild = children[0];
    for (auto &child : children)
        if (child->ni > optimalChild->ni)
            optimalChild = child;
        
    return optimalChild->decision;
}

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
	return {-1, -1};
}

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
    return C2 * minDist[1] - (1 - C2) * minDist[0];
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

int calGoodPos(int board[][SIZE]) {
    int cnt = 0;
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            if (board[i][j] == 0) cnt++;
    return cnt;
}

int main()
{
#ifndef _BOTZONE_ONLINE
	freopen("in.txt", "r", stdin);
    freopen("out.txt", "w", stdout);
#endif // _BOTZONE_ONLINE

    int x, y, n;//记录目前棋盘有几个子
    //恢复目前的棋盘信息
    cin >> n;
    for (int i = 0; i < n - 1; i++){
        cin >> x >> y; if (x != -1) board[x][y] = -1;	//对方
        cin >> x >> y; if (x != -1) board[x][y] = 1;	//我方
    }
    cin >> x >> y;  if (x != -1) board[x][y] = -1;	//对方
    else {
        cout << 1 << ' ' << 2 << endl;          //限制先手
        return 0;
    }
    //此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋

    if(board[1][2] == 1) isFirstPlayer= true; // 我方为红方
    redValue = isFirstPlayer ? 1 : -1;  // 判断红方是board[][]中值为1的点还是-1的点

    // 启发式规则
    Position herPos = Heuristic(n, x, y);
    if (herPos.x != -1 && herPos.y != -1) {
        cout << herPos.x << ' ' << herPos.y << endl;
        return 0;
    }


    // MinmaxTree* minmaxRoot = new MinmaxTree();
    // expand(minmaxRoot);
    // if (isFirstPlayer) AlphaBeta(minmaxRoot, MAX_DEPTH, -INF, INF, Red);
    // else AlphaBeta(minmaxRoot, MAX_DEPTH, -INF, INF, Blue);
    // Position bestAction = getOptimalDecision(minmaxRoot);

    srand(time(0));
    MCTtree* gameRoot = new MCTtree(board, {-1, -1}, nullptr);

    // Monte Carlo Tree Search
    while(clock() - start < timeout)
    {
        MCTtree *curNode = gameRoot;
        while (curNode->getChild().size() > 0) curNode = curNode->SelectMaxUCBNode();
        if (curNode->getNi() <= CNT) curNode->rollout();
        else curNode->Expand();
    }

    Position bestAction= gameRoot->getOptimalDecision();
    cout << bestAction.x << ' ' << bestAction.y << endl;

    delete(gameRoot);

    return 0;
}