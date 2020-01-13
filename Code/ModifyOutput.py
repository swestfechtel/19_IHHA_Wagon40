import pandas
import os


def line_prepender(filename, line):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        f.write(line.rstrip('\r\n') + '\n' + content)


def generate_header(path):
    tmp = pandas.read_csv(path)
    col_cnt = tmp.shape[1]
    line = 'Timestamp'
    for i in range(col_cnt - 1):
        line += f",Wagon {i}"

    return line


dir_cnt = 0
directories = ''

for _, dirnames, _ in os.walk('output', topdown=False):
    dir_cnt += len(dirnames)
    directories = dirnames

print('Modifying output files..')

for dirname in directories:
    print(dirname)
    line_prepender(f"output/{dirname}/Distance.csv", 'Timestamp,Distance')
    line_prepender(f"output/{dirname}/Velocity.csv", 'Timestamp,Velocity')
    line_prepender(f"output/{dirname}/Force.csv", generate_header(f"output/{dirname}/Force.csv"))
    line_prepender(f"output/{dirname}/Pressure.csv", generate_header(f"output/{dirname}/Pressure.csv"))
    line_prepender(f"output/{dirname}/Acceleration_neg.csv", generate_header(f"output/{dirname}/Acceleration_neg.csv"))


print('Done modifying output files.')
