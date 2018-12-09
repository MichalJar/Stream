#!/home/michal/anaconda3/bin/python3

import sys
import random
import subprocess
import os

fname = "f"
def create_data(n):
    with open(fname, mode="w") as f:
        for x in range(0,n):
            a = random.random() * 10000.0
            b = random.random() * 10000.0
            f.write("{},{}\n".format(a,b))

mode = sys.argv[1]
if mode == "SG":
    n = int(sys.argv[2])   
    x_start = int(sys.argv[3])
    x_end   = int(sys.argv[4])
    step = 5

    # chunk_size = int(sys.argv[3])

    create_data( n*2 )
    cwd = os.getcwd()

    for x in range(x_start, x_end+1, 5):
        chunk_num = int(n / x)
        chunk_size = int(x)
        #print("chunk_num: {}, chunk_size: {}".format(chunk_num,chunk_size))
        command = "{}/subgraphmain {} {} {}".format(cwd,fname, int(chunk_num), int(chunk_size))
        process = subprocess.Popen(command.split())
        process.wait()
                
elif mode == "CT":
    model_num = int(sys.argv[2])
    create_data( model_num )
    cwd = os.getcwd()
    print(cwd)

    command = "{}/clustreemain {}".format(cwd,fname)
    process = subprocess.Popen(command.split())

#cwd = os.getcwd()
#print(cwd)

#command = "{}/clustreemain {}".format(cwd,fname)
#process = subprocess.Popen(command.split())
