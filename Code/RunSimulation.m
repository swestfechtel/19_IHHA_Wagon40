%% Initialise
clear all, close all, clc

%% Vars
m = 90000; % Masse
v0 = 100/3.6; % Ausgangsgeschwindigkeit
efficiency = 0.95; % efficiency of braking force generation
trackgradient = 0; % Steigung/Gefaelle der Stecke
trainpower = m; % Zughakenleistung

%% Constants

Fb = -1*m; %?
RBD = 5; %Regelbetriebsdruck
VBD = 3.5; %Vollbremsdruck
%Brake pipe
l = 18; %Wagonlänge?
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
tmax = 200;
nmax = 400;
t = linspace(0, tmax, nmax);
u = [5*ones(8*nmax/40,1); 3.5*ones(24*nmax/40,1); 5*ones(8*nmax/40,1)];
f = [m*ones(8*nmax/40,1); 0*ones(24*nmax/40,1); m*ones(4*nmax/40,1); 0*ones(4*nmax/40,1)];
%u = [5*ones(nmax/4,1); 3.5*ones(3*nmax/4,1)];
simin.time = t;
simin.signals.values = u;%[-300*ones(nmax,1)];

simin1.time = t;
simin1.signals.values = f;


sim('Simulation.slx')
V = [velocity.Time, velocity.Data];
V1 = [velocity1.Time, velocity1.Data];
F = [force.Time, force.Data];
F1 = [force1.Time, force1.Data];
P = [pressure.Time, pressure.Data];
P1 = [pressure1.Time, pressure1.Data];
writematrix(V,'Velocity.csv');
writematrix(V1,'Velocity1.csv');
writematrix(F,'Force.csv');
writematrix(F1,'Force1.csv');
writematrix(P,'Pressure.csv');
writematrix(P1,'Pressure1.csv');