import torch
import torch.nn as nn
import numpy as np
import json


boardH = 11
boardW = 11
input_c=3

class CNNLayer(nn.Module):
    def __init__(self, in_c, out_c):
        super().__init__()
        self.conv=nn.Conv2d(in_c,
                      out_c,
                      3,
                      stride=1,
                      padding=1,
                      dilation=1,
                      groups=1,
                      bias=False,
                      padding_mode='zeros')
        self.bn= nn.BatchNorm2d(out_c)

    def forward(self, x):
        y = self.conv(x)
        y = self.bn(y)
        y = torch.relu(y)
        return y


class ResnetLayer(nn.Module):
    def __init__(self, inout_c, mid_c):
        super().__init__()
        self.conv_net = nn.Sequential(
            CNNLayer(inout_c, mid_c),
            CNNLayer(mid_c, inout_c)
        )

    def forward(self, x):
        x = self.conv_net(x) + x
        return x

class Outputhead_v1(nn.Module):

    def __init__(self,out_c,head_mid_c):
        super().__init__()
        self.cnn=CNNLayer(out_c, head_mid_c)
        self.valueHeadLinear = nn.Linear(head_mid_c, 3)
        self.policyHeadLinear = nn.Conv2d(head_mid_c, 1, 1)

    def forward(self, h):
        x=self.cnn(h)

        # value head
        value = x.mean((2, 3))
        value = self.valueHeadLinear(value)

        # policy head
        policy = self.policyHeadLinear(x)
        policy = policy.squeeze(1)

        return value, policy



class Model_ResNet(nn.Module):

    def __init__(self,b,f):
        super().__init__()
        self.model_name = "res"
        self.model_size=(b,f)

        self.inputhead=CNNLayer(input_c, f)
        self.trunk=nn.ModuleList()
        for i in range(b):
            self.trunk.append(ResnetLayer(f,f))
        self.outputhead=Outputhead_v1(f,f)

    def forward(self, x):
        if(x.shape[1]==2):#global feature is none
            x=torch.cat((x,torch.zeros((1,input_c-2,boardH,boardW))),dim=1)
        h=self.inputhead(x)

        for block in self.trunk:
            h=block(h)

        return self.outputhead(h)


ModelDic = {
    "res": Model_ResNet
}

class Board:
    def __init__(self):
        self.board = np.zeros(shape=(2, boardH, boardW))
    def self_transpose(self):
        self.board=self.board.swapaxes(1,2)
    def play(self,x,y,color):
        self.board[color,y,x]=1
    def isLegal(self,x,y):
        if(self.board[0,y,x] or self.board[1,y,x]):
            return False
        return True

#读取神经网络
def loadModel(file_path):
    if(file_path=='random'):
        return 'random'
    modeldata = torch.load(file_path,map_location=torch.device('cpu'))

    model_type=modeldata['model_name']
    model_param=modeldata['model_size']
    model = ModelDic[model_type](*model_param)
    model.load_state_dict(modeldata['state_dict'])
    model.eval()
    return model

if __name__ == '__main__':
    file_path="data/hex_tt8.pth"
    model=loadModel(file_path)

    board = Board()

    # 解析读入的JSON
    full_input = json.loads(input())
    if "data" in full_input:
        my_data = full_input["data"]  # 该对局中，上回合该Bot运行时存储的信息
    else:
        my_data = None


    # 分析自己收到的输入和自己过往的输出，并恢复状态
    all_requests = full_input["requests"]
    all_responses = full_input["responses"]

    #检查强制落子
    myColor=1 # default white
    last_request=all_requests[-1]
    if('forced_x' in last_request):
        forced_x=last_request['forced_x']
        forced_y=last_request['forced_y']
        my_action = {"x": forced_x, "y": forced_y}
        print(json.dumps({
            "response": my_action
        }))
        exit(0)
    #重建棋盘
    else:
        if(int(all_requests[0]['x']) == -1):
            all_requests=all_requests[1:]
            myColor = 0 #black
        for r in all_requests:
            loc=int(r['x']) * 11+int(r['y'])
            board.play(r['x'],r['y'],1)
        for r in all_responses:
            loc=int(r['x']) * 11+int(r['y'])
            board.play(r['x'],r['y'],0)

    # 随机落子 -----------------------------------------------------------------------------------------------------------
    if(model=='random'):
        x = np.random.randint(0,boardW)
        y = np.random.randint(0,boardH)
        i=0
        while board.board[0,y,x] == 1 or board.board[1,y,x] == 1:
            i = i + 1
            x = np.random.randint(0,boardW)
            y = np.random.randint(0,boardH)
            if (i > 10000):  # have bug
                break
        my_action = {"x": x, "y": y}
        print(json.dumps({
            "response": my_action
        }))
        exit(0)

    # 神经网络计算 --------------------------------------------------------------------------------------------------------
    # 保证己方是x方向连接，对方是y方向连接
    shouldTranspose= myColor==0
    if(shouldTranspose):
        board.self_transpose()


    nnboard = torch.FloatTensor(board.board)
    nnboard.unsqueeze_(0)
    v,p = model(nnboard)


    policytemp=0.3

    policy=p.detach().numpy().reshape((-1))
    policy=policy-np.max(policy)
    for i in range(boardW*boardH):
        if not board.isLegal(i%boardW,i//boardW):
            policy[i]=-10000
    policy=policy-np.max(policy)
    for i in range(boardW*boardH):
        if(policy[i]<-1):
            policy[i]=-10000
    policy=policy/policytemp
    probs=np.exp(policy)
    probs=probs/sum(probs)
    for i in range(boardW*boardH):
        if(probs[i]<1e-3):
            probs[i]=0

    action = int(np.random.choice([i for i in range(boardW*boardW)],p=probs))
    if(shouldTranspose):
        my_action = {"y": action % boardW, "x": action // boardW}
    else:
        my_action = {"x": action % boardW, "y": action // boardW}
    print(json.dumps({
        "response": my_action
    }))