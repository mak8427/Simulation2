import matplotlib.pyplot as plt
import numpy as np
import os
import pandas as pd
import json

# Read in the data
with open('Defines.json') as d:
    Defines = json.load(d)
n_agents=Defines['n_agents']
data = pd.read_csv('cmake-build-debug/data.csv')
for i in range(n_agents):
    plt.plot(data['iteration'], data['Pop 1 food value'])
plt.show()
