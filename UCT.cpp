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

const int SIZE = 11, CNT = 60;
const double C = 0.6, TIMELIMIT = 0.96;
const int start=clock(), timeout = (int)(TIMELIMIT*(double)CLOCKS_PER_SEC);

bool isFirstPlayer;     // true为红方（连接上下边界），false为蓝方（连接左右边界）
int board[SIZE][SIZE];  // 我方 1，对方 -1，无子0
int dx[6] = {-1, -1, 0, 1, 1, 0}, dy[6] = {0, 1, 1, 0, -1, -1};  // 方向向量
bool st[SIZE][SIZE];
int number;
int N;

struct Position {
    int x, y;
};

class MCTtree {
public:      // API
    MCTtree(int[][SIZE], Position, MCTtree*);
    MCTtree* SelectMaxUCBNode();
    void Expand();
    void rollout();
    Position getOptimalDecition();

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


Position MCTtree::getOptimalDecition()
{
    MCTtree* optimalChild = children[0];
    for (auto &child : children)
        if (child->ni > optimalChild->ni)
            optimalChild = child;
        
    return optimalChild->decision;
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

    Position bestAction= gameRoot->getOptimalDecition();
    cout << bestAction.x << ' ' << bestAction.y << endl;

    return 0;
}