import http.server
import socketserver

import os, sys

PORT = 9450
class Handler(http.server.BaseHTTPRequestHandler):
	def do_GET(self):
		board = self.path
		if board.startswith('/'):
			board = board[1:]
		stdout.write(board)
		stdout.flush()
		pos = []
		while True:
			c = stdin.read(1)
			pos.append(c)
			if c == '\n':
				break
		pos = "".join(pos)
		self.send_response(200)
		self.send_header('Access-Control-Allow-Origin', '*')
		self.end_headers()
		self.wfile.write(pos.encode('utf-8'))

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
