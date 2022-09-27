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
level=200

#plot 1
fig=plt.figure(figsize=(16,9))
xnew = list(range( data['Pop '+str(0)+" Bought"][level], data['Pop '+str(n_Pop)+" Bought"][level]))
y=[]
x=[]
print("test")
for i in range(0, n_Pop):
    y.append(data['Pop '+str(i)+" food value"][level])
    x.append(data['Pop '+str(i)+" Bought"][level])

#find the index of repeated values in x
index = [i for i in range(len(x)) if x.count(x[i]) > 1]
#remove the repeated values
x_ = [x[i] for i in range(len(x)) if i not in index]
y_ = [y[i] for i in range(len(y)) if i not in index]

xnew=range(x_[0],x_[-1])
f=interp1d(x_, y_, kind='cubic')
#f2= interp1d(z, q, kind='cubic')
#for i in range(10,n_Pop,10):
#    plt.plot(data['iteration'], data['Pop '+str(i)+" food value"], label='Pop '+str(i))
plt.plot(x, y, 'o', xnew, f(xnew),'-',)

plt.savefig('cmake-build-debug/Analysis.png')



#matplot lib animation
#new df
fig = plt.figure(figsize=(16, 9))
#def buildmebarchart(i=int):


import matplotlib.animation as ani
#animator = ani.FuncAnimation(fig, buildmebarchart, interval = 50)
#animator.save(r'cmake-build-debug/myfirstAnimation.gif')

from celluloid import Camera
from matplotlib import cm

camera = Camera(plt.figure())
for _ in range(200):
    level = _+1
    # plot 1
    y = []
    x = []
    for i in range(0, n_Pop):
        y.append(data['Pop ' + str(i) + " food value"][level])
        x.append(data['Pop ' + str(i) + " Bought"][level])

    index = [i for i in range(len(x)) if x.count(x[i]) > 12]
    x_ = [x[i] for i in range(len(x)) if i not in index]
    y_ = [y[i] for i in range(len(y)) if i not in index]
    print(index)
    plt.yscale('log')
    plt.xscale('log')
    #plt.ylim(y[-1]+y[-1]/10+50,y[1]-y[1]/10-50)
    #plt.xlim(x[1]-x[1]/10-50, x[-1]+x[-1]/10)
    #xnew = range(x_[0], x_[-1])
    f = interp1d(x, y, kind='cubic')
    plt.plot(x, y, 'o',color='blue')
    camera.snap()
animation = camera.animate()
animation.save('cmake-build-debug/animation.gif', writer='Pillow', fps=100)


