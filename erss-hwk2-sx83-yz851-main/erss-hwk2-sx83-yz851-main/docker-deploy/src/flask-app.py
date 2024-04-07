# -*- coding: utf-8 -*-
from flask import Flask, make_response, request
from datetime import datetime, timedelta
import email.utils as eut

app = Flask(__name__)

ETAG_VALUE = 0
LAST_MODIFIED = datetime.utcnow()

@app.route('/cache')
def cache():
    resp = make_response('This response is cacheable.\n')
    resp.headers['Cache-Control'] = 'max-age=10'
    return resp

@app.route('/no-store')
def no_store():
    resp = make_response('This response should not be cached.\n')
    resp.headers['Cache-Control'] = 'no-store'
    return resp

@app.route('/private')
def private():
    resp = make_response('This response is private.\n')
    resp.headers['Cache-Control'] = 'private'
    return resp

@app.route('/expires-success')
def expires_success():
    resp = make_response('This response has an Expires header and it is in the future.\n')
    expires_time = datetime.utcnow() + timedelta(seconds=60) - timedelta(seconds=4 * 3600)
    resp.headers['Expires'] = expires_time.strftime("%a, %d %b %Y %H:%M:%S GMT")
    return resp

@app.route('/expires-failed')
def expires_failed():
    resp = make_response('This response has an Expires header and it is in the past.\n')
    expires_time = datetime.utcnow() - timedelta(seconds=120) - timedelta(seconds=4 * 3600)
    resp.headers['Expires'] = expires_time.strftime("%a, %d %b %Y %H:%M:%S GMT")
    return resp

@app.route('/etag-variable')
def etag_variable():
    global ETAG_VALUE
    ETAG_VALUE += 1
    etag = str(ETAG_VALUE)

    if 'If-None-Match' in request.headers and request.headers['If-None-Match'] == etag:
        return ('', 304)
    resp = make_response('ETag will change for each request.\n')
    resp.headers['ETag'] = etag
    return resp

@app.route('/etag-constant')
def etag_constant():
    etag = "constant-etag"

    if 'If-None-Match' in request.headers and request.headers['If-None-Match'] == etag:
        return ('', 304)
    resp = make_response('ETag is constant for all requests.\n')
    resp.headers['ETag'] = etag
    return resp

@app.route('/last-modified-variable')
def last_modified_variable():
    last_modified = datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")

    if 'If-Modified-Since' in request.headers:
        if_modified_since = datetime(*eut.parsedate(request.headers['If-Modified-Since'])[:6])
        if if_modified_since >= datetime.utcnow():
            return ('', 304)
    resp = make_response('Last-Modified will change for each request.\n')
    resp.headers['Last-Modified'] = last_modified
    return resp

@app.route('/last-modified-constant')
def last_modified_constant():
    global LAST_MODIFIED
    last_modified = LAST_MODIFIED.strftime("%a, %d %b %Y %H:%M:%S GMT")

    if 'If-Modified-Since' in request.headers:
        if_modified_since = datetime(*eut.parsedate(request.headers['If-Modified-Since'])[:6])
        if if_modified_since >= LAST_MODIFIED:
            return ('', 304)
    resp = make_response('Last-Modified is constant for all requests.\n')
    resp.headers['Last-Modified'] = last_modified
    return resp


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
