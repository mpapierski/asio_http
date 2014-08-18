'''
Integration tests for the HTTP server. Each of test case runs
against real server that implements few features that outputs
various request data to JSON.
'''
import sys
import signal
import unittest
import subprocess
import json
import urllib2

_, TEST_SERVER_EXE = sys.argv


class BetterHTTPErrorProcessor(urllib2.BaseHandler):

    def http_error_405(self, request, response, code, msg, hdrs):
        return response


class Response(object):

    def __init__(self, request):
        self.request = request
        self.data = None

    @property
    def url(self):
        return self.request.geturl()

    @property
    def response_code(self):
        return self.request.getcode()

    @property
    def json(self):
        if self.data is None:
            self.data = self.request.read()
        return json.loads(self.data)

    @property
    def content(self):
        if self.data is None:
            self.data = self.request.read()
        return self.data


def json_request(url, data=None, headers={}):
    request = urllib2.Request(url, data=data)
    for k, v in headers.items():
        request.add_header(k, v)
    r = urllib2.urlopen(request)
    return Response(r)


class ServerTestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        opener = urllib2.build_opener(BetterHTTPErrorProcessor)
        urllib2.install_opener(opener)

    def setUp(self):
        '''Start server instance
        '''
        self.process = subprocess.Popen([TEST_SERVER_EXE],
                                        stdout=subprocess.PIPE)
        b = self.process.stdout.readline()
        json_data = json.loads(b)
        self.assertEqual(json_data['type'], 'listen')
        self.url = 'http://127.0.0.1:{0}'.format(json_data['data']['port'])

    def tearDown(self):
        self.process.send_signal(signal.SIGTERM)
        self.process.wait()

    def test_get(self):
        r = json_request(self.url + '/get')
        self.assertEqual(r.url, self.url + '/get')
        self.assertEqual(r.response_code, 200)
        self.assertEqual(r.json['url'], '/get')

    def test_get_with_post(self):
        r = json_request(self.url + '/get', data='Hello world')
        self.assertEqual(r.url, self.url + '/get')
        self.assertEqual(r.response_code, 405)
        self.assertIn('Method Not Allowed', r.content)

    def test_post(self):
        r = json_request(self.url + '/post', data='Hello world')
        self.assertEqual(r.url, self.url + '/post')
        self.assertEqual(r.response_code, 200)
        self.assertEqual(r.json['url'], '/post')
        self.assertEqual(r.json['data'], 'Hello world')

if __name__ == '__main__':
    unittest.main(argv=sys.argv[:1])
