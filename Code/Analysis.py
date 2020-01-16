import pandas
import matplotlib.pyplot as plt

tmp = pandas.read_csv('output/run1/Force.csv')
df = tmp.set_index('Timestamp')

print(df.shape)
print(df)

plt.figure()
df.plot()
plt.autoscale()
plt.show()
