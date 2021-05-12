import os

for i in[4,16]:
    for j in[1,8]:
        if(i==4):
            if os.path.isfile('host_file_4_'+str(j)):
                os.system('rm host_file_4_'+str(j))
            os.system("python script.py 2 2 "+str(j))
        else:
            if os.path.isfile('host_file_16_'+str(j)):
                os.system('rm host_file_16_'+str(j))
            os.system("python script.py 4 4 "+str(j))
