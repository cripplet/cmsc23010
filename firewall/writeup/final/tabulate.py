#!/bin/python

log = open('log.txt', 'r')
out = open('out.txt', 'r')

result = {}

out_lines = out.readlines()
log_lines = log.readlines()

log.close()
out.close()

for i in xrange(len(out_lines)):
	exp = out_lines[i][:-1].split(' ')[-1][-1]
	l = log_lines[i][:-1].split('\t')[1:]

	if len(l) == 7:
		(T, n, W, uniform, iter, mode, time) = l
	else:
		(T, n, W, uniform, iter, mode, time, _) = l

	key = (exp, T, n, W, uniform, mode)
	if key in result:
		result[key] += float(time) / 5.
	else:
		result[key] = float(time) / 5.

fp = open('result.csv', 'a')

for (key, val) in result.iteritems():
	(exp, T, n, W, uniform, mode) = key
	if not exp == '2':
		speedup = result[(exp, T, n, W, uniform, '1')] / result[key]
	else:
		speedup = 1
	fp.write("\t".join(str(x) for x in key) + "\t" + str(val) + "\t" + str(speedup) + "\n")

fp.close()
