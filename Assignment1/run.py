import os
import sys
import re
import fileinput

if os.path.isfile('output.txt'):
    os.system('rm output.txt')

os.system("make")

print("generating hosts file")
os.system("$HOME/UGP/allocator/src/allocator.out 64 8")

for j in [16,36,49,64]:
    print ("No of Processes = {}".format(j))
    if os.path.isfile('result{}.txt'.format(j)):
        os.system('rm result{}.txt'.format(j))
    for i in range(4,11):
        data=1<<i
        print("calculating {}^2".format(data))
        for jn in range(5):
            os.system("mpiexec -np {} -f hosts ./src {} 50 >> result{}.txt".format(j,data,j))
    
    count=0
    data_arr=[16,32,64,128,256,512,1024]
    for line in fileinput.input(['./result{}.txt'.format(j)],inplace=True):
        if (count%3==0):
            sys.stdout.write('Direct {} {l}'.format(data_arr[int (count/15)],l=line))
        elif (count%3==1):
            sys.stdout.write('Packed {} {l}'.format(data_arr[int(count/15)],l=line))
        else:
            sys.stdout.write('Vector {} {l}'.format(data_arr[int (count/15)],l=line))
        count=count+1

os.system('python3 plot.py')

