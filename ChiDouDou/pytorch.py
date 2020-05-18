
import torch
import torch.utils.data as data
import torchvision.transforms as transforms
import numpy as np
import re
import json



class MyDataset(data.Dataset):
    # wall = {
    # 'emptyWall': 0,
    # 'wallNorth': 1,
    # 'wallEast': 2,
    # 'wallSouth': 4,
    # 'wallWest': 8,
    # 'generator': 16
    # }
    useFulPlayers =[
        "5e9086246cfafa05779b7e63",
        "5e95ccaceef4d305643e777a",
        "5827061f06d742eb45a3ff3c",
        "59941f85579ce9447f9c1032",
        "582962d6ef6400e971c6a135",
        "58271db006d742eb45a403e2",
        "5825f82069e937e742d429b9",
        "582721be06d742eb45a404b8",
        "58ee17a6d9c62b2c44cdb8c4",
        "58271dd406d742eb45a403ee",
        "5e899d6341dfbb5e9196d362",
        "5820722c5aab41d534febff5",
        "5861315e6d6c3d654a247e28",
        "5820880a5aab41d534fec8d8",
        "5827412106d742eb45a409e8",
        "5827197a06d742eb45a40307",
        "581fc5ce5aab41d534fe7d6a",
        "5e6b1da1d53c5473d60e973e",
        "5820812c5aab41d534fec604",
        "582563857865521b372ef869",
    ]
    def __init__(self, file_list, basis_url):
        self.matchList=[]
        for fileUrl in file_list:
            file = open(basis_url+"/"+fileUrl)
            for matchString in file.read().split("\n"):
                match =  json.loads(matchString)
                isUsful = False
                for useFulplayer in useFulPlayers:
                    if(useFulplayer in match["players"]):
                        isUsful = True
                if(isUsful):
                    self.matchList.append(match)
                    
                    
    def __getitem__(self, idx):
        input = []
        output = []
        
        return input, output


    def __len__(self):
        return self.num_samples
