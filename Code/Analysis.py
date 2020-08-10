import numpy as np
import pandas as pd

# id,time(i),f,p,v,a,d,wagon_ids,grad,ft,fc
columns = ['id', 'time']

for i in range(40):
	columns.append("F" + str(i))

for i in range(40):
	columns.append("P" + str(i))

columns.append('velocity')

for i in range(40):
	columns.append("a" + str(i))

columns.append(distance)

for i in range(40):
	columns.append("Wagon" + str(i))

columns.append('gradient')
columns.append('Ft')
columns.append('fc')

df = pd.read_csv('output/output.tsv', names=columns, memory_map=True)
df.info(verbose=True)