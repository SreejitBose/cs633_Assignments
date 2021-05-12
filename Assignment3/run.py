import os
import sys

nodes=[1,2]
cores=[1,2,4]

if os.path.isfile('output.txt'):
    os.system('rm output.txt')

os.system("make")

print("generating hosts file")
os.system("python3 generate_hostfile.py")

for n in nodes:
    for c in cores:
        proc=n*c
        if(n==1):
            os.system("mpiexec -np "+str(proc)+" ./src tdata.csv > output.txt")
        else:
            os.system("mpiexec -np "+str(proc)+" -f hostfile_2_{} ./src tdata.csv > output.txt".format(c))

