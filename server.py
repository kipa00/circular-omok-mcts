import http.server
import socketserver

import os, sys

PORT = 9450
class Handler(http.server.BaseHTTPRequestHandler):
	def do_GET(self):
		path = self.path
		if path.startswith('/'):
			path = path[1:]
		if len(path) == 81 and all(x in '012' for x in path):
			stdout.write(path)
			stdout.flush()
			pos = []
			while True:
				c = stdin.read(1)
				pos.append(c)
				if c == '\n':
					break
			data = "".join(pos).encode('utf-8')
			response = 200
		else:
			if not path:
				path = 'index.html'
			response = 404
			try:
				with open(path, 'rb') as f:
					data = f.read()
				response = 200
			except:
				with open('not-found.html', 'rb') as f:
					data = f.read()
		self.send_response(200)
		self.send_header('Access-Control-Allow-Origin', '*')
		self.end_headers()
		self.wfile.write(data)

def main():
	global stdin
	global stdout
	r1, w1 = os.pipe()
	r2, w2 = os.pipe()
	pid = os.fork()
	if pid == 0:
		os.dup2(r1, 0)
		os.dup2(w2, 1)
		os.close(w1)
		os.close(r2)
		os.execv('./mcts', ['./mcts'])
		sys.exit(-1)
	else:
		stdin, stdout = r2, w1
		os.close(r1)
		os.close(w2)
		stdin = os.fdopen(stdin)
		stdout = os.fdopen(stdout, 'w')
		with socketserver.TCPServer(("", PORT), Handler) as httpd:
			print('serving at port', PORT)
			httpd.serve_forever()

if __name__ == '__main__':
	main()
