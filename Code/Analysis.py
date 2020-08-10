import pandas

# id,time(i),f,p,v,a,d,wagon_ids,grad,ft,fc
columns = ['id', 'time']

for i in range(40):
	columns.append(f"F{i}")

for i in range(40):
	columns.append(f"P{i}")

columns.append('velocity')

for i in range(40):
	columns.append(f"a{i}")

columns.append(distance)

for i in range(40):
	columns.append(f"Wagon{i}")

columns.append('gradient')
columns.append('Ft')
columns.append('fc')

df = pandas.read_csv('output/output.tsv', names=columns, memory_map=True)
df.info(verbose=True)