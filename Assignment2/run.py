import os
import sys
import re
import fileinput

if os.path.isfile('output_Bcast.txt'):
    os.system('rm output1_Bcast.txt')
if os.path.isfile('output_Reduce.txt'):
    os.system('rm output_Reduce.txt')
if os.path.isfile('output_Gather.txt'):
    os.system('rm output_Gather.txt')
if os.path.isfile('output_AlltoAllv.txt'):
    os.system('rm output_AlltoAllv.txt')

def run(data,nodes,ppn,flag):
    if (nodes==4):
        total_process=nodes*ppn
        print("mpiexec -np "+str(total_process)+" -f hostfile_4_"+str(ppn)+" ./code "+str(data)+" 2 2 "+str(ppn)+" "+flag+"")
        os.system("mpiexec -np "+str(total_process)+" -f hostfile_4_"+str(ppn)+" ./code "+str(data)+" 2 2 "+str(ppn)+" "+flag+"")
    else:
        total_process=nodes*ppn
        print("mpiexec -np "+str(total_process)+" -f hostfile_16_"+str(ppn)+" ./code "+str(data)+" 4 4 "+str(ppn)+" "+flag+"")
        os.system("mpiexec -np "+str(total_process)+" -f hostfile_16_"+str(ppn)+" ./code "+str(data)+" 4 4 "+str(ppn)+" "+flag+"")

os.system("make")

print("generating hosts file")
os.system("python3 generate_hostfile.py")
for i in[1,2,3,4]:
    for execution in range(10):
        for P in [4, 16]:
            for ppn in [1, 8]:
                for D in [16, 256, 2048]:
                    run(D,P,ppn,str(i))

os.system('python3 plot.py Bcast')
os.system('python3 plot.py Reduce')
os.system('python3 plot.py Gather')
os.system('python3 plot.py AlltoAllv')
