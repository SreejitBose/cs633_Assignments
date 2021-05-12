import os
import sys


for p in [1,2,4]:
    if os.path.isfile('hostfile_2_'+str(p)):
        os.system('rm hostfile_2_'+str(p))
    os.system("python script.py 1 2 "+str(p))    

