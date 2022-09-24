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
#Level
level=20

#plot 1
fig=plt.figure(figsize=(16,9))
xnew = list(range( data['Pop '+str(0)+" Bought"][level], data['Pop '+str(n_Pop)+" Bought"][level]+1))
y=[]
x=[]
print("test")
for i in range(0, n_Pop):
    y.append(data['Pop '+str(i)+" food value"][level])
    x.append(data['Pop '+str(i)+" Bought"][level])


#f = interp1d(y, q, kind='cubic')
#remove dupluiactes from y
y = list(dict.fromkeys(y))
x = list(dict.fromkeys(x))
f=interp1d(x, y, kind='quadratic',)
#f2= interp1d(z, q, kind='cubic')
#for i in range(10,n_Pop,10):
#    plt.plot(data['iteration'], data['Pop '+str(i)+" food value"], label='Pop '+str(i))
plt.plot(x, y, 'o', xnew, f(xnew), '-')



plt.savefig('cmake-build-debug/Analysis.png')








#Plot a scatter plot wtch each Pop's food value for the first iteration
fig=plt.figure(figsize=(16,9))
q=0
list1=list(range(n_Pop-2))
for i in range(0,30):

    plt.scatter(data['Pop '+str(i)+" food value"][level],data['Pop '+str(i)+" Bought"][level], label='Pop '+str(i))
    q+=1
    #plot with a log scale
plt.savefig('cmake-build-debug/Analysis2.png')

