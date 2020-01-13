import csv
import random

with open('pool.csv', 'w', newline='') as pool:
    fieldnames = ['Wagon_ID', 'Mass', 'Braking_eff']
    writer = csv.DictWriter(pool, fieldnames=fieldnames)

    writer.writeheader()
    for i in range(0,500): # create 500 randomized wagons
        mass = random.randrange(12000, 90000, 100) # create random wagon mass between 12 t and 90 t with step size 100
        braking_eff = round(random.uniform(0.75, 0.95),2) # create random braking efficiency as uniform distribution between 75 % and 95 %
        writer.writerow({'Wagon_ID': i, 'Mass': mass, 'Braking_eff': braking_eff})
