import argparse as ap
from random import randint

def main():
	parser = ap.ArgumentParser()
	parser.add_argument('-n', type=int, nargs=1, help="size of array; strictly positive")
	parser.add_argument('-i', type=int, nargs=1, help="loop ID for multiple executions; default to 0")
	args = parser.parse_args()

	dim = args.n[0]
	if args.i:
		id = args.i[0]
	else:
		id = 0

	arr = [ [ 0 for x in xrange(dim) ] for x in xrange(dim) ]

	for i in xrange(dim):
		for j in xrange(i):
			val = randint(0, 101)
			if val == 101:
				val = 100000
			arr[i][j] = val
			arr[j][i] = val

	filename = 'output_data/in_i%i_n%i.txt' % (id, dim)
	f = open(filename, 'w+')
	f.write(str(dim) + "\n")
	for i in xrange(dim):
		f.write(" ".join([ str(x) for x in arr[i] ]) + "\n")
	f.close()

main()
