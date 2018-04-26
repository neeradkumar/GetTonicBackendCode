HTTP_PORT = 5000

#import shared library with computations
from ctypes import *
libg = CDLL("getTonicDrone.so")

#flask server
from flask import Flask, url_for, send_from_directory, request, abort
import logging, os
from werkzeug import secure_filename
app = Flask(__name__)
UPLOAD_FOLDER = '/uploads'
ALLOWED_EXTENSIONS = set(['txt', 'pdf', 'png', 'jpg', 'jpeg', 'gif','wav','mp3','3gp'])
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

import unicodedata

@app.route("/")
def hello():
    return "Get Tonic code online! Use /uploadAudioFile to calculate"


@app.route('/getTonicDrone',methods=['GET','POST'])
def getTonicDrone():
    if request.method == 'POST':
        if 'file_name' in request.form and 'meta_data' in request.form:
            filename = str(request.form.get('file_name'))
            metadata = int(request.form.get('meta_data'))
            seconds = str(request.form.get('seconds'))
            percentage = str(request.form.get('percentage'))
            method = str(request.form.get('method'))

            try:
                seconds = int(seconds)
            except:
                seconds = 90
            try:
                percentage = int(percentage)
            except:
                percentage = 10
        
            if method == 'Peak picking method':
                method = 1
            else:
                method = 2

        	try:
                #print 'File: '+filename+' metadata: '+str(metadata)+' seconds: '+str(seconds)+ ' percentage: '+str(percentage)+' method: '+str(method) 
        		calculated_pitch = str(libg.GetTonicDrone('uploads/'+filename,metadata,seconds,percentage,method))
        		print filename+' '+calculated_pitch
        		return calculated_pitch
        	except Exception as e:
        		abort(500)
        else:
        	abort(400)
    else:
        return "USE POST TO GET TONIC"

def shutdown_server():
    func = request.environ.get('werkzeug.server.shutdown')
    if func is None:
        raise RuntimeError('Not running with the Werkzeug Server')
    func()

@app.route('/shutdown', methods=['GET'])
def shutdown():
    shutdown_server()
    return 'Server shutting down...'


app.run(debug=True, host='0.0.0.0',port=HTTP_PORT, threaded=True)
