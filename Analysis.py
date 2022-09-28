import matplotlib.pyplot as plt
import numpy as np
import os
import pandas as pd
import json
from scipy.interpolate import interp1d
from scipy.optimize import curve_fit
from scipy import stats
import imageio
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
import warnings


warnings.filterwarnings('ignore')

camera = Camera(plt.figure())
def func(x, a, b, c,):

    return a * np.exp(-b * x) + c

def objective(x, a, b, c, d):
	return a * np.sin(b - x) + c * x**2 + d

for _ in range(200):
    level = _+1
    # plot 1
    y = []
    x = []
    for i in range(0, n_Pop):
        y.append(data['Pop ' + str(i) + " food value"][level])
        x.append(data['Pop ' + str(i) + " Bought"][level])

    popt, pcov = curve_fit(func, x, y)

    res = stats.linregress(x, y)
    plt.plot(x, y, 'o',color='blue')
    plt.plot(x, func(np.array(x), *popt), 'r-',
             label='fit: a=%5.3f, b=%5.3f, c=%5.3f' % tuple(popt))
    #plot a line with value data["Food Value"][level]
    plt.plot(x, [data["food value"][level]]*len(x), 'g-')
    plt.legend()


    plt.savefig('i_gif/image'+str(_)+'.png', dpi=100)
    plt.clf()
#animation = camera.animate()
#animation.save('cmake-build-debug/animation.gif', writer='Pillow', fps=100)

def make_gif(input_folder, save_filepath):
    episode_frames = []
    time_per_step = 0.1
    for root, _, files in os.walk(input_folder):
        file_paths = [os.path.join(root, file) for file in files]
        # sorted by modified time
        file_paths = sorted(file_paths, key=lambda x: os.path.getmtime(x))
        episode_frames = [imageio.imread(file_path)
                          for file_path in file_paths if file_path.endswith('.png')]
    episode_frames = np.array(episode_frames)
    imageio.mimsave(save_filepath, episode_frames, duration=time_per_step)


make_gif('./i_gif/', './test.gif')


fig.show()