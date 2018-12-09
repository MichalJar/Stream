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
    model_num = int(sys.argv[2])
    # chunk_size = int(sys.argv[3])

    create_data( model_num*3 )
    cwd = os.getcwd()
    print(cwd)

    # 100 na 100
    step = int( (model_num - 10) / 30 )
    for n in range(10, model_num, step):
        for x in range(10, model_num, step):
            if x < n:
                chunk_num = int(n / x)
                chunk_size = int(x)

                #print("n: {}, x: {}, chunk_size: {}, chunk_num: {}".format(n, x, chunk_size, chunk_num))

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
