HTTP_PORT = 5000

#import shared library with computations
from ctypes import *
libg = CDLL("getTonicDrone.so")


import unicodedata
def getTonicDrone(filename, metadata):
	try:

        #print 'File: '+filename+' metadata: '+str(metadata)+' seconds: '+str(seconds)+ ' percentage: '+str(percentage)+' method: '+str(method) 
		calculated_pitch = str(libg.GetTonicDrone(filename,metadata,120,10,2))
		print filename+' '+calculated_pitch
		return calculated_pitch
	except Exception as e:
		print e

dirname = "testing_audio"
import glob, os
os.chdir(".")
times = {}
for file in glob.glob("*.wav"):
    #file = 'testing_audio/'+file
    print file
    import time
    start = time.clock()
    i = getTonicDrone(file,1)
    times[file] = (i, time.clock() - start)

for t in times.keys():
    print t,times[t][0],times[t][1]