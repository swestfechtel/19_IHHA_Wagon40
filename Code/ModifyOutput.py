import pandas
import os
from numpy import genfromtxt


def line_prepender(filename, line):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        f.write(line.rstrip('\r\n') + '\n' + content)


def generate_header(path, ids):
    # tmp = pandas.read_csv(path)
    # col_cnt = tmp.shape[1]
    line = 'Timestamp'
    # for i in range(col_cnt - 1):
    for i in ids:
        line += f",{i}"

    return line


def read_ids(path):
    a = genfromtxt(path, delimiter=',', dtype=int)
    return a


dir_cnt = 0
directories = ''

for _, dirnames, _ in os.walk('output', topdown=False):
    dir_cnt += len(dirnames)
    directories = dirnames

print('Modifying output files..')

for dirname in directories:
    print(dirname)
    ids = read_ids(f"output/{dirname}/ids.csv")
    line_prepender(f"output/{dirname}/Distance.csv", 'Timestamp,Distance')
    line_prepender(f"output/{dirname}/Velocity.csv", 'Timestamp,Velocity')
    line_prepender(f"output/{dirname}/Force.csv", generate_header(f"output/{dirname}/Force.csv", ids))
    line_prepender(f"output/{dirname}/Pressure.csv", generate_header(f"output/{dirname}/Pressure.csv", ids))
    line_prepender(f"output/{dirname}/Acceleration_neg.csv", generate_header(f"output/{dirname}/Acceleration_neg.csv", ids))


print('Done modifying output files.')
