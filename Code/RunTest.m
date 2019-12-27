%% Initialise
%clear all, close all, clc
clear all
clc

%% Vars
m = 90000; % Masse
%v0 = 100/3.6; % Ausgangsgeschwindigkeit
efficiency = 0.95; % efficiency of braking force generation
trackgradient = 0; % Steigung/Gefaelle der Stecke
Ft = 360000;
fc = 0.45;

%% Constants

Fb = -1*m; %?
RBD = 5; %Regelbetriebsdruck
VBD = 3.5; %Vollbremsdruck
%Brake pipe
l = 18; %Wagonl�nge?
c = 250; %Ausbreitungsgeschwindigkeit 250km/h
%flow resistance
%wagon position

%Distributor valve
tf = 4; %filling time 4s +-1
tl = .1; %?
%brake cylinder pressure pc = 3.8bar +0.2bar,-0.1bar

%Braking force generation
%efficiency [0.75, 0.95], uniform distribution
%brake radius
%friction coefficient

%wheel/rail surface
%?

p0 = 0; %Ausgangsdruck?
s0 = 0; %?
Pres = 0/1000*[5.7/771 0 1.6]; %Strahl formula for m/s velocity

alpha = 0.9; %?

BPnum = [0.3 alpha]; %?
BPden = [1 alpha]; %?

%% Run Simulation
tmax = 3600;
nmax = tmax * 2;
t = linspace(0, tmax, nmax);
u = [20*ones(800*2,1); 27*ones(800*2,1); 15*ones(600*2,1); 10*ones(400*2,1); 22*ones(1000*2,1)];
simin.time = t;
simin.signals.values = u;

sim('Simulation_v2.slx')