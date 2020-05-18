import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
import math
from torch.autograd import Variable
from dataset import MyDataset
from pacman2 import Pacman

# 超参数
BATCH_SIZE = 32
LR = 0.01  # learning rate
# 强化学习的参数
EPSILON = 0.6  # greedy policy
GAMMA = 0.9  # reward discount
TARGET_REPLACE_ITER = 50   # target update frequency
MEMORY_CAPACITY = 500
N_STATES = 8
N_ACTIONS = 9


class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.conv1 = nn.Conv2d(N_STATES, 16, 3, stride=1, padding=0)
        self.conv2 = nn.Conv2d(16, 16, 3, stride=1, padding=0)

        self.fc1 = nn.Linear(16*7*7, 128)
        self.fc2 = nn.Linear(128, 64)
        self.choose = nn.Linear(64, 9)

    def forward(self, x):
        x = self.conv1(x)
        x = F.leaky_relu(x)
        x = self.conv2(x)
        x = F.leaky_relu(x)
        x = self.fc1(x.view(x.size(0), -1))
        x = self.fc2(x)
        x = self.choose(x)
        return x


class DQN(object):
    def __init__(self):
        self.eval_net, self.target_net = Net(), Net()
        # 记录学习到多少步
        self.learn_step_counter = 0  # for target update
        self.memory_counter = 0  # for storing memory
        # 初始化memory
        self.stateMemory = np.zeros((MEMORY_CAPACITY, 2, N_STATES, 11, 11))
        self.arMemory = np.zeros((MEMORY_CAPACITY, 2))
        self.optimizer = torch.optim.Adam(self.eval_net.parameters(), lr=LR)
        self.loss_func = nn.MSELoss()

    def choose_action(self, x):
        x = Variable(torch.unsqueeze(torch.FloatTensor(x), 0))
        if np.random.uniform() < EPSILON:
            action_value = self.eval_net.forward(x)
            action = torch.max(action_value, 1)[1].data.numpy()[0]
        else:  # random
            action = np.random.randint(0, N_ACTIONS)
        return action

    # s:当前状态， a:动作, r:reward奖励, s_:下一步状态
    def store_transaction(self, s, a, r, s_):
        # replace the old memory with new memory
        index = self.memory_counter % MEMORY_CAPACITY
        self.stateMemory[index][0] = np.array(s)
        self.stateMemory[index][1] = np.array(s_)
        self.arMemory[index] = np.array([a, r])
        self.memory_counter += 1

    def learn(self):
        # target net update
        self.learn_step_counter += 1
        if self.learn_step_counter % TARGET_REPLACE_ITER == 0:
            self.target_net.load_state_dict(self.eval_net.state_dict())
            print("sync network parameters")
            torch.save(self.target_net.state_dict(), "./model/model.pt")

        sample_index = np.random.choice(MEMORY_CAPACITY, BATCH_SIZE)
        b_states_memory = self.stateMemory[sample_index, :]
        b_ar_memory = self.arMemory[sample_index, :]
        b_s = Variable(torch.FloatTensor(b_states_memory[:, 0]))
        b_a = Variable(torch.LongTensor(
            b_ar_memory[:, 0]))
        b_r = Variable(torch.FloatTensor(
            b_ar_memory[:, 1]))
        b_s_ = Variable(torch.FloatTensor(b_states_memory[:, 1]))
        q_next = self.target_net(b_s_).detach()
        q_target = b_r + GAMMA * q_next.max(1)[0]
        q_eval = self.eval_net(b_s).gather(1, b_a.view(1, BATCH_SIZE).t())
        loss = self.loss_func(q_eval.view(BATCH_SIZE), q_target)
        self.optimizer.zero_grad()
        loss.backward()

        self.optimizer.step()
        return str(loss.data)


def paddZero(states, height, width):
    pad = nn.ZeroPad2d(padding=(math.ceil((11-width)/2), math.floor((11-width)/2),
                                math.ceil((11-height)/2), math.floor((11-height)/2)))
    res = []
    for i in range(8):
        res.append(pad(torch.tensor(states[i])).tolist())
    return res


dqn = DQN()
file_list = [
    "./data/101-200.matches",
    # "./data/201-300.matches",
    # "./data/301-400.matches",
    # "./data/401-500.matches",
    # "./data/501-600.matches",
    # "./data/601-700.matches",
    # "./data/701-800.matches",
    # "./data/801-900.matches",
    # "./data/901-1000.matches",
    # "./data/1001-1100.matches",
    # "./data/1101-1200.matches",
    # "./data/1201-1300.matches",
    # "./data/1301-1400.matches",
    # "./data/1401-1500.matches",
    # "./data/1501-1600.matches",
    # "./data/1601-1700.matches",
    # "./data/1701-1800.matches",
    # "./data/1801-1900.matches",
    # "./data/1901-2000.matches"
]
print('\nlet us begin')
dataSet = MyDataset(file_list)
matchCount = 0
batchMatchCount = 0
batchTurnCount = 0
while True:

    # 遍历每张地图
    # for data in dataSet.matchList:
    data = dataSet.matchList[4]
    match = Pacman(data)

    height, width = match.getHeightAndWidth()
    states = [0 for i in range(4)]
    states_ = [0 for i in range(4)]
    actions = [0 for i in range(4)]
    me = [0 for i in range(4)]
    for pls_id in range(4):
        states[pls_id] = paddZero(match.getData(pls_id), height, width)
        actions[pls_id] = dqn.choose_action(states[pls_id])
        me[pls_id] = match.getRes(pls_id)
        # take action
    Continue = match.nextTurn(actions)
    batchTurnCount += 1
    for pls_id in range(4):
        states_[pls_id] = paddZero(match.getData(pls_id), height, width)
        me_ = match.getRes(pls_id)
        if me_.dead:
            reward = 0
        elif me_.isBig:
            reward = (me_.strength - me[pls_id].strength)*0.1
        else:
            reward = (me_.strength - me[pls_id].strength)*0.1
        dqn.store_transaction(
            states[pls_id], actions[pls_id], reward, states_[pls_id])
        if dqn.memory_counter % MEMORY_CAPACITY == 0:
            loss = dqn.learn()
            print("loss: " + loss + " in " +
                    str(dqn.memory_counter) + " turns")

    if not Continue:
        batchMatchCount += 1
        matchCount += 1
        if matchCount % 1000 == 0:
            print("end of match " + str(matchCount) +
                    " with avanage turnCount: " + str(batchTurnCount/batchMatchCount))
            batchMatchCount = 0
            batchTurnCount = 0
        continue
