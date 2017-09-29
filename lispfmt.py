#!/usr/bin/python
# formatting for lisp code
# 2 spaces per paren depth per line
import sys

def reformat(s):
	depth = 0

	# formatted string
	fmt = ""

	# remove leading whitespaces
	unfmt = ""
	for c in s:
		if c == '\t' or c == ' ' and hit == False:
			continue
		else:
			unfmt += c
			if c == '\n':
				hit = False
			else:				
				hit = True

	# add in proper indentation
	for c in unfmt:
		fmt += c 
		if c == '(':
			depth += 1
		elif c == ')':
			depth -= 1
		elif c == '\n':
			for x in range(0, depth):
				fmt += "  "

	return fmt

def format_file(filename):
	with open(filename) as f:
		return reformat(f.read())


if __name__ == "__main__":
	if len(sys.argv) > 1:
		for f in sys.argv[1:]:
			print(format_file(f))
	else:
		s = ""
		for line in sys.stdin:
			s += line
		print(reformat(s))