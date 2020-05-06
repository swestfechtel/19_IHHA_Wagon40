%% Initialise
%clear all, close all, clc
clear all
clc
warning ('off','all');

%% Constants

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
%s0 = 0; %?
Pres = 0/1000*[5.7/771 0 1.6]; %Strahl formula for m/s velocity

alpha = 0.9; %?

BPnum = [0.3 alpha]; %?
BPden = [1 alpha]; %?

%% Simulation input
tmax = 3600;
nmax = tmax * 2;
t = linspace(0, tmax, nmax);
u = [20*ones(800*2,1); 27*ones(800*2,1); 15*ones(600*2,1); 10*ones(400*2,1); 22*ones(900*2,1); 0*ones(100*2,1)];
simin.time = t;
simin.signals.values = u;


%% Vars
% m = 90000; % Masse
%v0 = 100/3.6; % Ausgangsgeschwindigkeit
%efficiency = 0.95; % efficiency of braking force generation
trackgradient = 0; % Steigung/Gefaelle der Stecke
%Ft = 360000; % traction force
%fc = 0.45;
% Fb = -1*m; %?
pool = readmatrix('pool.csv');
% num_wagons = 10;
mkdir output;
% for i = 1:1:40
% 	num_wagons = i;
% 	name_wagons = strcat('wagons',num2str(i));
% 	for j = 0.05:0.01:0.78
% 		fc = j;
% 		name_fc = strcat('fc',num2str(j));
% 		for k = 200000:1000:400000
% 			Ft = k;
% 			sim('Simulation_v2.slx');
% 			name = strcat(name_wagons,name_fc,'ft',num2str(k));
% 			WriteOutput(name,velocity,force,pressure,distance,acceleration_neg,ids,t,u,trackgradient,Ft,fc);
% 			fprintf('Run %s complete.\n', name);
% 		end
% 	end
% end

wagons = 1:1:40;
friction = 0.05:0.01:0.78;
tracforce = 200000:1000:400000;
track = 1;
allruns = 1;
for i = length(wagons):-1:1
    for j = length(friction):-1:1
        for k = length(tracforce):-1:1
            in(track) = Simulink.SimulationInput('Simulation_v2');
            in(track) = in(track).setVariable('num_wagons',wagons(i));
            in(track) = in(track).setVariable('fc',friction(j));
            in(track) = in(track).setVariable('Ft',tracforce(k));
            
            in(track) = in(track).setVariable('alpha',alpha);
            in(track) = in(track).setVariable('BPden',BPden);
            in(track) = in(track).setVariable('BPnum',BPnum);
            in(track) = in(track).setVariable('c',c);
            in(track) = in(track).setVariable('l',l);
            in(track) = in(track).setVariable('nmax',nmax);
            in(track) = in(track).setVariable('p0',p0);
            in(track) = in(track).setVariable('pool',pool);
            in(track) = in(track).setVariable('Pres',Pres);
            in(track) = in(track).setVariable('RBD',RBD);
            in(track) = in(track).setVariable('simin',simin);
            in(track) = in(track).setVariable('t',t);
            in(track) = in(track).setVariable('tf',tf);
            in(track) = in(track).setVariable('tl',tl);
            in(track) = in(track).setVariable('tmax',tmax);
            in(track) = in(track).setVariable('trackgradient',trackgradient);
            in(track) = in(track).setVariable('u',u);
            in(track) = in(track).setVariable('VBD',VBD);
            
            Ft(track) = tracforce(k);
            fc(track) = friction(j);
            
            track = track + 1;        
           % fprintf('Run %i complete.\n',track);
        end
    end

    parpool(10);
    out = parsim(in,'ShowProgress','on');
    for l = 1:1:length(out)
   	allruns = allruns + 1;
        name = strcat('run',num2str(allruns));
        WriteOutput(name,out(l).get('velocity'),out(l).get('force'),out(l).get('pressure'),out(l).get('distance'),out(l).get('acceleration_neg'),out(l).get('ids'),t,u,trackgradient,Ft(l),fc(l));
    end
    track = 1;
    delete(gcp('nocreate'));
    fprintf('Run %i complete.\n',i);
end

%myCluster = parcluster('local');
%myCluster.NumWorkers = 20;
%saveProfile(myCluster);
%myCluster

%parpool(20);
