import os
import sys
import re
import fileinput

def plot():
     no_package=0    
     pand = False
     mat = False
     pack_resp=os.popen("pip3 list --format=columns").read()
     pack_match=re.search(pattern="pandas",string=pack_resp)
     if(pack_match is None):
          no_package=1
          pand = True

     pack_match=re.search(pattern="matplotlib",string=pack_resp)
     if(pack_match is None):
          no_package=1
          mat = True

     if (no_package==0):
          import pandas as pd
          import matplotlib
          matplotlib.use('Agg')
          import matplotlib.pyplot as plt
          if os.path.isfile('output.txt'):
               df = pd.read_csv('output.txt', squeeze = True,header=None)
               df = df.rename(columns={0:"Datapoints",1:"Process",2:"Direct",3:"Packed",4:"Vector"})
               
               plt.rcParams.update({'font.size': 22})
               
               for j in [16,36,49,64]:
                    print("creating process {} plot".format(j))
                    a = df[df["Process"]==j].filter(["Datapoints","Direct","Packed","Vector"])
                    m = df[df["Process"]==j].filter(["Datapoints","Direct","Packed","Vector"]).groupby("Datapoints").mean()

                    fig,ax=plt.subplots(figsize=(20, 15))

                    m.filter(["Direct"]).rename(index={16:1, 32:2, 64:3, 128:4, 256:5, 512:6, 1024:7}).plot.line(ax=ax)
                    a.filter(["Datapoints","Direct"]).boxplot(by= "Datapoints",ax=ax)

                    m.filter(["Packed"]).rename(index={16:1, 32:2, 64:3, 128:4, 256:5, 512:6, 1024:7}).plot.line(ax=ax)
                    a.filter(["Datapoints","Packed"]).boxplot(by= "Datapoints",ax=ax)

                    box3 = m.filter(["Vector"]).rename(index={16:1, 32:2, 64:3, 128:4, 256:5, 512:6, 1024:7}).plot.line(ax=ax)
                    box3 = a.filter(["Datapoints","Vector"]).boxplot(by= "Datapoints",ax=ax)

                    box3.set_xlabel("Datapoints(squared)")
                    box3.set_ylabel("Time(sec)")

                    plt.title("{} Process".format(j))
                    plt.xticks([1,2,3,4,5,6,7],[16,32,64,128,256,512,1024],rotation=20)
                    plt.savefig('plot{}.jpg'.format(j))
                    plt.close()
               return True
          else:
               print("file 'output.txt' not found")
               return False
     else:
          print("Your system don't have following python library installed:")
          if pand:
               print("\npandas\tTo install run: pip3 install pandas")
          if mat:
               print("\nmatplotlib\nTo install run: pip3 install matplotlib")
          print("Above python library are pre-installed on following(as of now): \ncsews1\tcsews2\tcsews3\tcsesw34")
          return False

if plot():
     print("plot successful")
else:
     print("plot unsuccessful")
