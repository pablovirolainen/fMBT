import sys
import BaseHTTPServer
import SimpleHTTPServer

_html_basepage='''
<html>
    <head>
        <script language="JavaScript" type="text/javascript">
        %s
        </script>
    </head>
    <body>
        <div id="newdiv">
        </div>
        fMBTweb
    </body>
</html>
''' % (file("fmbtweb.js").read(),)

class MyRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def __init__(self, *args):
        SimpleHTTPServer.SimpleHTTPRequestHandler.__init__(self, *args)

    def log_request(self, *args):
        pass # don't want to log send_responses

    def send_ok(self, content):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-length", str(len(content)))
        self.end_headers()
        self.wfile.write(content)

    def do_GET(self):
        if self.path == '/':
            self.send_ok(_html_basepage)
            return
        elif self.path.startswith('/fMBTweb.'):
            print "fMBTweb response:", self.path[9:]
            next_cmd = raw_input() # get new action here
            self.send_ok(next_cmd)
            return
        print "serving a file:", self.path
        SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

def _start_http_server(port):
    server = BaseHTTPServer.HTTPServer(('', int(port)), MyRequestHandler)
    while 1:
        server.handle_request()
    return server

# take
# - port as an optional argument
# - list of javascript files to be included to HTML
# - alternatively HTML string which must include fmbtweb.js script

if __name__ == '__main__':
    if len(sys.argv) == 1:
        import random
        port = random.randint(10000,50000)
    else:
        port = int(sys.argv[1])

    browser_start_cmd = "sleep 1; chromium http://localhost:%s" % (port,)

    import subprocess
    browser = subprocess.Popen(browser_start_cmd, shell=True)
    _start_http_server(port)
