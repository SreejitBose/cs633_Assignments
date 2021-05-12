#!/usr/bin/env python
# coding: utf-8


import os
import sys
import pandas as pd
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt

sns.set()

flag=sys.argv[1]
f=open('output_'+flag+'.txt','r')

demo_input_format = pd.DataFrame.from_dict({
    "D": [],
    "P": [],
    "ppn": [],
    "mode": [],  # 1 --> optimized, 0 --> standard
    "time": [],
})


for execution in range(10):
    for P in [4, 16]:
        for ppn in [1, 8]:
            for D in [16, 256, 2048]:
                # Change with the actual data
                demo_input_format = demo_input_format.append({
                    "D": D, "P": P, "ppn": ppn, "mode": 'Standard', "time": float(list(f.readline()[:-1].split(","))[5])
                }, ignore_index=True)
                demo_input_format = demo_input_format.append({
                    "D": D, "P": P, "ppn": ppn, "mode": 'Optimized', "time": float(list(f.readline()[:-1].split(","))[5])
                }, ignore_index=True)

demo_input_format["(P, ppn)"] = list(map(lambda x, y: ("(" + x + ", " + y + ")"), map(str, demo_input_format["P"]), map(str, demo_input_format["ppn"])))

sea=sns.catplot(x="(P, ppn)", y="time", data=demo_input_format, kind="bar", col="D", hue="mode",ci=68,legend=False)
plt.legend(loc='upper left')
plt.suptitle(flag,fontsize=10)
plt.tight_layout()
plt.savefig('plot_'+flag+'.jpg')






