
import json 
import math
import numpy as np

class Player:
    def __init__(self, idi=0, x=0, y=0):        
        self.id = idi        
        self.strength = 1        
        self.x = x        
        self.y = y        
        self.isBig = 0        
        self.dead = False
class Generator:
    def __init__(self, x = 0, y = 0):        
        self.x = x        
        self.y = y
class Pacman:       

    def actionValid(self,pl, move):    
        if move == -1:        
            return True    
        if move >= 4:        
            return move < 8 and self.players[pl].strength > self.skillCost    
        return move >= 0 and move < 4 and not(self.gameStatic[self.players[pl].x][self.players[pl].y] & (1 << move))
    def nextTurn(self,actionsInput):
        actions = actionsInput    
        # invalid input    
        for i in range(4):        
            if not(self.players[i].dead):            
                ac = actions[i]            
                if ac == -1:                
                    continue            
                if not(self.actionValid(i, ac)):                
                    self.gameContent[self.players[i].x][self.players[i].y] &= (~(1 << self.players[i].id))                
                    self.players[i].strength = 0                
                    self.players[i].dead = True                
                       
                    self.aliveNum -= 1            
                elif ac < 4:                
                    nx = (self.players[i].x + self.dx[ac] + self.height) % self.height                
                    ny = (self.players[i].y + self.dy[ac] + self.width) % self.width                
                    nxtp = self.gameContent[nx][ny]                
                    if nxtp & (1 | 2 | 4 | 8):                    
                        for j in range(4):                        
                            if (nxtp & (1 << j)) and self.players[j].strength > self.players[i].strength:         
                                actions[i] = -1 
                                
        for i in range(4):        
            if self.players[i].dead or actions[i] == -1 or actions[i] >= 4:            
                continue         
            ac = actions[i]        
            self.gameContent[self.players[i].x][self.players[i].y] &= (~(1 << i))        
            npx = (self.players[i].x + self.dx[ac] + self.height) % self.height
            npy = (self.players[i].y + self.dy[ac] + self.width) % self.width
            self.players[i].x, self.players[i].y = npx, npy        
            self.gameContent[npx][npy] |= (1 << i)
        for i in range(4):        
            if self.players[i].dead:            
                continue       
            px, py = self.players[i].x, self.players[i].y        
            player, cnt = 0, 0        
            con_players = [0] * 4        
            for player in range(4):            
                if self.gameContent[px][py] & (1 << player):                
                    con_players[cnt] = player                
                    cnt += 1        
            if cnt > 1:            
                for k in range(cnt):                
                    for j in range(cnt - k - 1):                    
                        if self.players[con_players[j]].strength < self.players[con_players[j + 1]].strength:                        
                            con_players[j], con_players[j + 1] = con_players[j + 1], con_players[j]            
                beg = 0 
                for beg in range(1, cnt):                
                    if self.players[con_players[beg - 1]].strength > self.players[con_players[beg]].strength:     
                        break            
                    ltstren = 0            
                    for k in range(beg, cnt):                
                        pid = con_players[k]                
                        self.gameContent[self.players[pid].x][self.players[pid].y] &= (~(1 << pid))                
                        self.players[pid].dead = True                
                        drop = self.players[pid].strength / 2                
                        ltstren += drop                
                        self.players[pid].strength = math.ceil(self.players[pid].strength - drop)                
                        self.aliveNum -= 1            
                    inc = int(ltstren / beg)            
                    for k in range(beg):                
                        pid = con_players[k]                
                        self.players[pid].strength += inc 
            for i in range(4):        
                if self.players[i].dead or actions[i] < 4:            
                    continue        
                self.players[i].strength -= self.skillCost        
                dirc = actions[i] - 4        
                r, c, pl = self.players[i].x, self.players[i].y, 0
                while not(self.gameStatic[r][c] & (1 << dirc)):            
                    r = (r + self.dx[dirc] + self.height) % self.height            
                    c = (c + self.dy[dirc] + self.width) % self.width                        
                    if r == self.players[i].x and c == self.players[i].y:                
                        break            
                    if (self.gameContent[r][c] & 15):                
                        for pl in range(4):
                            if (self.gameContent[r][c] & (1 << pl)):
                                self.players[pl].strength -= self.skillCost * 1.5                        
                                self.players[i].strength += self.skillCost * 1.5  
        for i in range(4):        
            if self.players[i].dead or self.players[i].strength > 0:            
                continue        
            self.gameContent[self.players[i].x][self.players[i].y] &= ~(1 << i)        
            self.players[i].dead = True        
            self.players[i].strength = 0        
            self.aliveNum -= 1     
        self.generatorLeftTurns -= 1 
        if self.generatorLeftTurns == 0:        
            self.generatorLeftTurns += 20        
            for i in range(4):
                genPic[self.generators[i].x][self.generators[i].y]  =  self.generatorLeftTurns           
                for d in range(8):                
                    r = (self.generators[i].x + self.dx[d] + self.height) % self.height                
                    c = (self.generators[i].y + self.dy[d] + self.width) % self.width                
                    if (self.gameStatic[r][c] & 16) or (self.gameContent[r][c] & (16 | 32)):   
                        continue                
                    self.gameContent[r][c] |= 16                             
                    self.smallCount += 1
        for i in range(4):        
            if self.players[i].dead:            
                continue        
            pr, pc = self.players[i].x, self.players[i].y        
            if self.gameContent[pr][pc] & 15 & (~(1 << i)):            
                continue        
            if (self.gameContent[pr][pc] & 16):            
                self.gameContent[pr][pc] &= (~16)            
                self.players[i].strength += 1            
                self.smallCount -= 1        
            elif self.gameContent[pr][pc] & 32:            
                self.gameContent[pr][pc] &= (~32)            
                if self.players[i].isBig == 0:                
                    self.players[i].strength += 10            
                self.players[i].isBig += 10 
        for i in range(4):       
            if self.players[i].dead:            
                continue        
            if self.players[i].isBig > 0:            
                self.players[i].isBig -= 1            
                if self.players[i].isBig == 0:                
                    self.players[i].strength -= 10
        for i in range(4):        
            if self.players[i].dead or self.players[i].strength > 0:            
                continue        
            self.gameContent[self.players[i].x][self.players[i].y] &= (~(1 << i))        
            self.players[i].dead = True        
            self.players[i].strength = 0
            self.aliveNum -= 1   
        self.turnId += 1
        if self.aliveNum <= 1:        
            for i in range(4):            
                if not(self.players[i].dead):                
                    self.players[i].strength += self.smallCount        
            return False    
        if self.turnId >= 100:        
            return False    
        return True     
    def __init__(self,data): 
        self.dx = [-1, 0, 1, 0, -1, 1, 1, -1]
        self.dy = [0, 1, 0, -1, 1, 1, -1, -1]
        self.players = [Player() for i in range(4)] 
        self.generators = [Generator() for j in range(4)] 
        self.aliveNum = 4 
        self.smallCount = 0 
        self.turnId = 0
        self.generateTurnCount = 0
        self.generatorLeftTurns=20
        self.skillCost = 4
        staticData = data["log"][0]["output"]["content"]["0"]
        self.gameStatic = staticData["static"]
        self.gameContent = staticData["content"]
        self.width = staticData["width"]
        self.height = staticData["height"]
        self.upPic = [[0]*self.width]*self.height
        self.downPic =[[0]*self.width]*self.height
        self.leftPic = [[0]*self.width]*self.height
        self.rightPic = [[0]*self.width]*self.height
        self.genPic = [[0]*self.width]*self.height
        for i in range(self.height):        
            for j in range(self.width):   
                if self.gameStatic[i][j] & 1:
                    self.upPic[i][j] = 1
                if self.gameStatic[i][j] & 2:
                    self.rightPic[i][j] = 1
                if self.gameStatic[i][j] & 4:
                    self.downPic[i][j] = 1 
                if self.gameStatic[i][j] & 8:  
                    self.leftPic[i][j] = 1                            
                if self.gameContent[i][j] & 16:                               
                    self.smallCount += 1            
                if self.gameContent[i][j] & 1:                
                    self.players[0].x, self.players[0].y, self.players[0].id = i, j, 0            
                if self.gameContent[i][j] & 2:                
                    self.players[1].x, self.players[1].y, self.players[1].id = i, j, 1            
                if self.gameContent[i][j] & 4:                
                    self.players[2].x, self.players[2].y, self.players[2].id = i, j, 2            
                if self.gameContent[i][j] & 8:               
                    self.players[3].x, self.players[3].y, self.players[3].id = i, j, 3                 
                if self.gameStatic[i][j] & 16: 
                    self.genPic[i][j] = 20                                     
                    self.generators[self.generateTurnCount].x, self.generators[self.generateTurnCount].y = i, j                
                    self.generateTurnCount += 1

    def getRes(self,m_id):
        return self.players[m_id]
    def getHeightAndWidth(self):
        return self.height, self.width
    def getData(self,m_id):  
        plsPic = [[0 for i in range(self.width)]for i in range(self.height)]
        mePic =[[0 for i in range(self.width)]for i in range(self.height)]
        frPic = [[0 for i in range(self.width)]for i in range(self.height)]
        for i in range(self.height):        
            for j in range(self.width): 
                if self.gameContent[i][j] & 16:                               
                    frPic[i][j] = 1
                if self.gameContent[i][j] & 32:                               
                    frPic[i][j] = 2
                if self.gameContent[i][j] & 1:           
                    if (not self.players[0].dead) and (not self.players[0].id==m_id):
                        plsPic[i][j] = self.players[0].strength
                    if self.players[0].id==m_id:
                        mePic[i][j] = self.players[0].strength          
                if self.gameContent[i][j] & 2:                
                    if (not self.players[1].dead) and (not self.players[1].id==m_id):
                        plsPic[i][j] = self.players[1].strength      
                    if self.players[1].id==m_id:
                        mePic[i][j] = self.players[1].strength                          
                if self.gameContent[i][j] & 4:  
                    if (not self.players[2].dead) and (not self.players[2].id==m_id):              
                        plsPic[i][j] = self.players[2].strength  
                    if self.players[2].id==m_id:
                        mePic[i][j] = self.players[2].strength                                  
                if self.gameContent[i][j] & 8:               
                    if (not self.players[3].dead) and (not self.players[3].id==m_id):              
                        plsPic[i][j] = self.players[3].strength
                    if self.players[3].id==m_id:
                        mePic[i][j] = self.players[3].strength
        return [self.upPic,self.rightPic,self.downPic,self.leftPic,self.genPic,frPic,plsPic,mePic]    
        
 
