import sys

s = "(car (cons 1 2))"
print(s)
top_list = []
cur_list = []
cur_token = ""
for i in s:
	cur_list = top_list
	if (i == '('): 
		cur_list = list()
		top_list.append(cur_list)
		
	elif (i == ')'):
		print()
	elif (i == ' '):
		cur_list.append(cur_token)
		cur_token = ""
	else:
		cur_token += i

print(top_list)
print(cur_token)
