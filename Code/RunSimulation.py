import matlab.engine
import matlab
import os

eng = matlab.engine.start_matlab()

# constants
rbd = float(5) 	# Regelbetriebsdruck 5 bar
vbd = float(3.5) 	# Vollbremsdruck 3.5 bar
l = float(18) 		# wagon length 18 m
c = float(250)		# propagation velocity 250 km/h
tf = float(4)		# filling time 4s
tl = float(.1) 	#
p0 = float(0)		# initial pressure
#Pres = eng.double(1/1000 * [5.7/771,0,1.6]) 	# Strahl formula for m/s velocity
Pres = matlab.double([0,0,0])
alpha = float(0.9) #
BPnum = matlab.double([0.3,alpha]) 	#
BPden = matlab.double([1,alpha]) 	#

# simulation input aka timeframe and track profile
tmax = 3600
nmax = tmax * 2
t = eng.linspace(float(0), float(tmax), float(nmax))
#u = matlab.double([20*eng.ones(800*2,1),27*eng.ones(800*2,1),15*eng.ones(600*2,1),10*eng.ones(400*2,1),22*eng.ones(1000*2,1)])
def helper(scalar, seconds):
	return list(map(lambda z: list(map(lambda y: y * scalar, z)), eng.ones(seconds*2,1)))


u = matlab.double(helper(20,800)+helper(27,800)+helper(15,600)+helper(10,400)+helper(22,1000))
#u.reshape(7200,1)
simin = {'time': t, 'signals': {'values': u}}

# variables
trackgradient = float(0)	# track gradient in ?
Ft = float(360000)			# traction force of train
fc = float(0.45)			# wheel surface friction coefficient

# wagon pool
pool = eng.readmatrix('pool.csv')		# read wagon pool (generate with CreatePool.py)
num_wagons = float(10)							# specifiy number of wagons
try:
	os.mkdir('output')					# create output directory if nonexistent
except:
	print("Creation of directory failed.")

# write values to workspace
eng.workspace['RBD'] = rbd
eng.workspace['VBD'] = vbd
eng.workspace['l'] = l
eng.workspace['c'] = c
eng.workspace['tf'] = tf
eng.workspace['tl'] = tl
eng.workspace['p0'] = p0
eng.workspace['Pres'] = Pres
eng.workspace['alpha'] = alpha
eng.workspace['BPnum'] = BPnum
eng.workspace['BPden'] = BPden
eng.workspace['tmax'] = tmax
eng.workspace['nmax'] = nmax
eng.workspace['t'] = t
eng.workspace['u'] = u
eng.workspace['simin'] = simin
eng.workspace['trackgradient'] = trackgradient
eng.workspace['Ft'] = Ft
eng.workspace['fc'] = fc
eng.workspace['pool'] = pool
eng.workspace['num_wagons'] = num_wagons


# run simulation
eng.sim('Simulation_v2.slx')

# read values from workspace
name = "test"
velocity = eng.workspace['velocity']
force = eng.workspace['force']
pressure = eng.workspace['pressure']
distance = eng.workspace['distance']
acceleration_neg = eng.workspace['acceleration_neg']
ids = eng.workspace['ids']

# write output files
eng.WriteOutput(name,velocity,force,pressure,distance,acceleration_neg,ids,t,u,trackgradient,Ft,fc)