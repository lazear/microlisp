
def push(a, b):
	a.append(b);

def pop(a):
	#last
	for x in a:
		if isinstance(x, list):
			print("LIST", x)
			last = x



def parse(string):
	exp = []
	ret = exp
	parent = exp
	word = ""
	#exp.append(string)
	print(string)
	for c in string:
		if c == '(':
			n = []
			exp.append(n)
			parent = exp
			exp = n
		elif c == ')':
			if word != "":
				exp.append(word)
				word = ""
			exp = parent
		else:
			if c == ' ':
				if word != "":
					exp.append(word)
					word = ""
			else:
				word += c

	print(ret)
	


parse("(define map (lambda (f a) (if (null? (cdr a ) ) (f (car a) ) (cons (f (car a) ) (map f (cdr a ) )) ) ))")

parse("(cons 1 2)")

parse("(cons '(1 2 3) '(4 5 6))")
parse("((x 1) 3)")
parse("(cons ((x 1) 3) (((y 2) (x 1)) 4))")