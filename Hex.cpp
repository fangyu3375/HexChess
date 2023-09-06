#include<iostream>
#include<stdlib.h>
#include<time.h>
#include<vector>
#include<algorithm>
#include<limits.h>
#include<math.h>
#include<chrono>
#include<string.h>
using namespace std;
const int SIZE = 11, CNT = 60;
const double C = 0.6;

const int start=clock(), timeout = (int)(0.96*(double)CLOCKS_PER_SEC);
bool isFirstPlayer;//为0蓝方，为1红方
int board[SIZE][SIZE]={0}; //本方1，对方-1，空白0
int disx[6]={-1,-1,0,1,1,0}, disy[6]={0,1,1,0,-1,-1};
int vis[SIZE][SIZE];
int visit[SIZE][SIZE];//判断谁赢的时候用
int WhocanwinLast;
int number;
int N;

struct Position {
    int x, y;
};

class MCTtree {
public:      // API
    MCTtree(int b[][11], Position, MCTtree*);
    MCTtree* SelectMaxUCBNode();
    void Expand();
    void rollout();
    Position getOptimalDecition();

public:     // getAttributes
    vector<MCTtree*>& getChild();
    double& getNi();

private:
    void backup(int);
    int get_available_pos(int avail_x[], int avail_y[]);
    void cloneBoardTo(int copy[][SIZE]);

private:    // Attributes
    MCTtree* parent;
    vector<MCTtree*> child;
    Position decision;
    int selfboard[11][11];  //每个节点存一个棋盘
    double value;  //ucb v
    double ni;     //ucb ni;
    double ucb;
};

vector<MCTtree*>& MCTtree::getChild() {
    return this->child;
}

double& MCTtree::getNi() {
    return this->ni;
}

MCTtree::MCTtree(int b[][11], Position pos, MCTtree* p)
    :parent(p), value(0), ni(0), ucb(INT_MAX), decision(pos) {
    memcpy(selfboard, b, sizeof(int) * 11 * 11);
}


void MCTtree::rollout()//模拟
{  
    int copy[11][11];
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

    memset(visit, 0, sizeof visit);

    WhocanwinLast=0;
    int f, ans = 0;
    if(isFirstPlayer==1){
        f=WhoCanWin1(copy);
    }
    else{
        f=WhoCanWin2(copy);
    }
    if(f==1) ans++;

    backup(ans);
}
void MCTtree::backup(int v)//回溯
{  
    N++;
    MCTtree* node=this;
    while(node){
        node->value+=v;
        node->ni++;
        node=node->parent;
    }
}

void MCTtree::Expand()
{
    int avail_x[SIZE * SIZE], avail_y[SIZE * SIZE];
    int cnt = get_available_pos(avail_x, avail_y);

    for(int i=0;i<cnt;i++){
        int x = avail_x[i], y = avail_y[i];
        MCTtree* c=new MCTtree(selfboard, {x, y},this);
        c->selfboard[x][y] = 1;
        child.push_back(c);
    }
    int k=rand()%cnt;
    child[k]->rollout();
}

MCTtree* MCTtree::SelectMaxUCBNode()
{
    for(int i=0;i<this->child.size();i++){
        if(child[i]->ni!=0){
            child[i]->ucb=double(child[i]->value)/(child[i]->ni)+C*sqrt(log(N)/child[i]->ni);
        }
    }
    int nb=0;
    double maxx=0;
    for(int i=0;i<child.size();i++){
        if(maxx<child[i]->ucb){
            nb=i;
            maxx=child[i]->ucb;
        }
    }
    return child[nb];
}

//得到当前棋局可走的合法位置表， 并返回个数
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

void MCTtree::cloneBoardTo(int copy[][SIZE])
{
    memcpy(copy, selfboard, sizeof(int) * SIZE * SIZE);
}	
Position MCTtree::getOptimalDecition()
{
    int res = 0, max_ni = 0;
    for (int i = 0; i < this->child.size(); ++i)
        if (this->child[i]->ni > max_ni)
        {
            max_ni = this->child[i]->ni;
            res = i;
        }
        
    return this->child[res]->decision;
}

bool avail(int x, int y)
{
    if (x >= 0 && x < SIZE && y >= 0 && y < SIZE && !board[x][y]) return true;
    return false;
}

void dfs1(int board[11][11],int x,int y){  
    visit[x][y]=1;
    if(board[x][y]==-1) return;
	if(x==10) {
	    WhocanwinLast=1;
	    return;
	}
	for(int i=0;i<6;i++){
		int newx=x+disx[i];
		int newy=y+disy[i];
		if(InTheHEX(newx,newy)&&visit[newx][newy]==0&&board[newx][newy]==1){
			dfs1(board,newx,newy);
		}
	}
}

void dfs2(int board[11][11],int x,int y){
    visit[x][y]=1;
    if(board[x][y]==-1) return;
	if(y==10) {
	    WhocanwinLast=1;
	    return;
	}
	for(int i=0;i<6;i++){
		int newx=x+disx[i];
		int newy=y+disy[i];
		if(InTheHEX(newx,newy)&&visit[newx][newy]==0&&board[newx][newy]==1){
			dfs2(board,newx,newy);
		}
	}
}
bool WhoCanWin2(int board[11][11]){   //后手判定胜负
	for(int i=0;i<11;i++){
		if(visit[i][0]==0&&board[i][0]==1){
			dfs2(board,i,0);
		}
	}
	return WhocanwinLast;
}
bool WhoCanWin1(int board[11][11]){   //先手判定胜负
	for(int i=0;i<11;i++){
		if(visit[0][i]==0&&board[0][i]==1){
			dfs1(board,0,i);
		}
	}
	return WhocanwinLast;
}

int main(void)
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
        cout << 1 << ' ' << 2 << endl;
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
        if (curNode->getNi() <= CNT - 1) curNode->rollout();
        else curNode->Expand();
    }

    Position bestAction= gameRoot->getOptimalDecition();
    cout << bestAction.x << ' ' << bestAction.y << endl;

    return 0;
}
