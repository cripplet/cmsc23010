from string import split
def main():
	fp = open("log.csv", "r")

	base = {}
	results = {}

	for line in fp.readlines():
		line_info = split(line, "\t")
		key = (line_info[4], line_info[5])
		if key in results:
			results[key] += float(line_info[6]) / 10.
		else:
			results[key] = float(line_info[6]) / 10.

	fp.close()
	fp = open("summary.csv", "w")

	fp.write("DIM\tTHREADS\texecution time\tspeedup\n")
	for ((dim, thread_count), val) in results.iteritems():
		fp.write(str(dim) + "\t" + str(thread_count) + "\t" + str(val) + "\t" + str(results[(dim, '0')] / val) + "\n")
	fp.close()

main()
