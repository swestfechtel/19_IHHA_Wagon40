import pandas
import matplotlib.pyplot as plt

tmp = pandas.read_csv('output/effi0.6/Force.csv')
df = tmp.set_index('Timestamp')

print(df.shape)
print(df)

plt.figure()
df.plot()
plt.show()
