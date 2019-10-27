import sys

filename=sys.argv[1]
outfilename=sys.argv[2]
lines = sorted(open(filename).readlines(), key=lambda line: float(line.split('\t')[4]), reverse=True)

with open(outfilename, 'a+') as f:
    for item in lines:
        f.write("%s" % item)
