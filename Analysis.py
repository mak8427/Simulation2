import matplotlib.pyplot as plt
import numpy as np
import os
import pandas as pd
import json
from scipy.interpolate import interp1d

# Read in the data
with open('Defines.json') as d:
    Defines = json.load(d)
n_Pop=Defines['n_agents']
data = pd.read_csv('cmake-build-debug/data.csv')
print(data)
fig=plt.figure(figsize=(16,9))
x = list(range(0, n_Pop))
y=[]
z=[]
for i in range(0, n_Pop):
    y.append(data['Pop '+str(i)+" food value"][10])
    z.append(data['Pop '+str(i)+" Bought"][10])


f = interp1d(y, z, kind='cubic')
#for i in range(10,n_Pop,10):
#    plt.plot(data['iteration'], data['Pop '+str(i)+" food value"], label='Pop '+str(i))


plt.plot(y, z, 'o', xnew, f(xnew), '-')
plt.legend()

plt.savefig('cmake-build-debug/Analysis.png')








#Plot a scatter plot wtch each Pop's food value for the first iteration
fig=plt.figure(figsize=(16,9))
q=0
list1=list(range(n_Pop-2))
for i in range(0,30):

    plt.scatter(data['Pop '+str(i)+" food value"][20],data['Pop '+str(i)+" Bought"][20], label='Pop '+str(i))
    q+=1
    #plot with a log scale
plt.savefig('cmake-build-debug/Analysis2.png')

