
import json 
from math import ceil


class Player:
    def __init__(self, idi=0, x=0, y=0):        
        self.id = idi        
        self.strength = 1        
        self.x = x        
        self.y = y        
        self.enhance = 0        
        self.dead = False
class Generator:
    def __init__(self, x = 0, y = 0):        
        self.x = x        
        self.y = y
class Pacman:       
    state_cnt = 0
    pls = [Player() for i in range(4)] 
    gts = [Generator() for j in range(4)] 
    gts_cnt = 0
    dx = [-1, 0, 1, 0, -1, 1, 1, -1]
    dy = [0, 1, 0, -1, 1, 1, -1, -1]
    width = 1
    height = 1
    actions = [-1] * 4 
    aliveCnt = 4 
    
    smallCnt = 0 
    turn_id = 0
    skill_cost = 4
    gameStatic = []
    gameContent = []
    my_id = 0
    generatorLeftTurns = 20 
    def actionValid(self,pl, move):    
        if move == -1:        
            return True    
        if move >= 4:        
            return move < 8 and pls[pl].strength > skill_cost    
        return move >= 0 and move < 4 and not(gameStatic[pls[pl].x][pls[pl].y] & (1 << move))
    def nextTurn(self):    
        # invalid input    
        for i in range(4):        
            if not(pls[i].dead):            
                ac = actions[i]            
                if ac == -1:                
                    continue            
                if not(actionValid(i, ac)):                
                    gameContent[pls[i].x][pls[i].y] &= (~(1 << pls[i].idi))                
                    pls[i].strength = 0                
                    pls[i].dead = True                
                       
                    aliveCnt -= 1            
                elif ac < 4:                
                    nx = (pls[i].x + dx[ac] + height) % height                
                    ny = (pls[i].y + dy[ac] + width) % width                
                    nxtp = gameContent[nx][ny]                
                    if nxtp & (1 | 2 | 4 | 8):                    
                        for j in range(4):                        
                            if (nxtp & (1 << j)) and pls[j].strength > pls[i].strength:         
                                actions[i] = -1 
                                
        for i in range(4):        
            if pls[i].dead or actions[i] == -1 or actions[i] >= 4:            
                continue         
            ac = actions[i]        
            gameContent[pls[i].x][pls[i].y] &= (~(1 << i))        
            npx = (pls[i].x + dx[ac] + height) % height
            npy = (pls[i].y + dy[ac] + width) % width
            pls[i].x, pls[i].y = npx, npy        
            gameContent[npx][npy] |= (1 << i)
        for i in range(4):        
            if pls[i].dead:            
                continue       
            px, py = pls[i].x, pls[i].y        
            player, cnt = 0, 0        
            con_players = [0] * 4        
            for player in range(4):            
                if gameContent[px][py] & (1 << player):                
                    con_players[cnt] = player                
                    cnt += 1        
            if cnt > 1:            
                for k in range(cnt):                
                    for j in range(cnt - k - 1):                    
                        if pls[con_players[j]].strength < pls[con_players[j + 1]].strength:                        
                            con_players[j], con_players[j + 1] = con_players[j + 1], con_players[j]            
                beg = 0 
                for beg in range(1, cnt):                
                    if pls[con_players[beg - 1]].strength > pls[con_players[beg]].strength:     
                        break            
                    ltstren = 0            
                    for k in range(beg, cnt):                
                        pid = con_players[k]                
                        gameContent[pls[pid].x][pls[pid].y] &= (~(1 << pid))                
                        pls[pid].dead = True                
                        drop = pls[pid].strength / 2                
                        ltstren += drop                
                        pls[pid].strength = ceil(pls[pid].strength - drop)                
                        aliveCnt -= 1            
                    inc = int(ltstren / beg)            
                    for k in range(beg):                
                        pid = con_players[k]                
                        pls[pid].strength += inc 
            for i in range(4):        
                if pls[i].dead or actions[i] < 4:            
                    continue        
                pls[i].strength -= skill_cost        
                dirc = actions[i] - 4        
                r, c, pl = pls[i].x, pls[i].y, 0
                while not(gameStatic[r][c] & (1 << dirc)):            
                    r = (r + dx[dirc] + height) % height            
                    c = (c + dy[dirc] + width) % width                        
                    if r == pls[i].x and c == pls[i].y:                
                        break            
                    if (gameContent[r][c] & 15):                
                        for pl in range(4):
                            if (gameContent[r][c] & (1 << pl)):
                                pls[pl].strength -= skill_cost * 1.5                        
                                pls[i].strength += skill_cost * 1.5  
        for i in range(4):        
            if pls[i].dead or pls[i].strength > 0:            
                continue        
            gameContent[pls[i].x][pls[i].y] &= ~(1 << i)        
            pls[i].dead = True        
            pls[i].strength = 0        
            aliveCnt -= 1     
        generatorLeftTurns -= 1 
        if generatorLeftTurns == 0:        
            generatorLeftTurns += 20        
            for i in range(4):            
                for d in range(8):                
                    r = (gts[i].x + dx[d] + height) % height                
                    c = (gts[i].y + dy[d] + width) % width                
                    if (gameStatic[r][c] & 16) or (gameContent[r][c] & (16 | 32)):   
                        continue                
                    gameContent[r][c] |= 16                             
                    smallCnt += 1
        for i in range(4):        
            if pls[i].dead:            
                continue        
            pr, pc = pls[i].x, pls[i].y        
            if gameContent[pr][pc] & 15 & (~(1 << i)):            
                continue        
            if (gameContent[pr][pc] & 16):            
                gameContent[pr][pc] &= (~16)            
                pls[i].strength += 1            
                smallCnt -= 1        
            elif gameContent[pr][pc] & 32:            
                gameContent[pr][pc] &= (~32)            
                if pls[i].enhance == 0:                
                    pls[i].strength += 10            
                pls[i].enhance += 10 
        for i in range(4):       
            if pls[i].dead:            
                continue        
            if pls[i].enhance > 0:            
                pls[i].enhance -= 1            
                if pls[i].enhance == 0:                
                    pls[i].strength -= 10
        for i in range(4):        
            if pls[i].dead or pls[i].strength > 0:            
                continue        
            gameContent[pls[i].x][pls[i].y] &= (~(1 << i))        
            pls[i].dead = True        
            pls[i].strength = 0
            aliveCnt -= 1   
        turn_id += 1
        if aliveCnt <= 1:        
            for i in range(4):            
                if not(pls[i].dead):                
                    pls[i].strength += smallCnt        
                    return False    
                if turn_id >= 100:        
                    return False    
                return True     
    def __init__(self,data,id):   
        my_id = id 
        for i in range(height):        
            for j in range(width):            
                if gameContent[i][j] & 16:                               
                    smallCnt += 1            
                if gameContent[i][j] & 1:                
                    pls[0].x, pls[0].y, pls[0].idi = i, j, 0            
                if gameContent[i][j] & 2:                
                    pls[1].x, pls[1].y, pls[1].idi = i, j, 1            
                if gameContent[i][j] & 4:                
                    pls[2].x, pls[2].y, pls[2].idi = i, j, 2            
                if gameContent[i][j] & 8:               
                    pls[3].x, pls[3].y, pls[3].idi = i, j, 3                 
                if gameStatic[i][j] & 16:                              
                    gts[gts_cnt].x, gts[gts_cnt].y = i, j                
                    gts_cnt += 1
    def getInMat(self,_turn, act):    
        ind = 0    
        feedlis = [0. for i in range(1223)]    
        feedlis[ind] = _turn / 100    
        ind += 1    
        # my_id    
        for i in range(4):        
            if i == my_id:            
                feedlis[ind] = 1        
            ind += 1    
        # strength    
        for i in range(4):        
            if pls[i].dead == False:            
                feedlis[ind] = pls[i].strength / 100        
            else:            
                feedlis[ind] = 0        
            ind += 1    
        # enhance    
        for i in range(4):        
            feedlis[ind] = pls[i].enhance        
            ind += 1    
        # each grid    
        for i in range(height):        
            for j in range(width):            
                # wall            
                for k in range(4):                
                    if gameStatic[i][j] & (1 << k):                    
                        feedlis[ind] = 1                
                    ind = ind + 1            
                # player            
                for k in range(4):                
                    if gameContent[i][j] & (1 << k):                    
                        feedlis[ind] = 1                
                    ind = ind + 1            
                # fruit            
                if gameContent[i][j] & 16:                
                    feedlis[ind] = 1            
                ind += 1            
                if gameContent[i][j] & 32:                
                    feedlis[ind] = 1            
                ind += 1    
        feedlis.append(act)    
        return feedlis
 
