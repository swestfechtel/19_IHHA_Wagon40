numchannel = 1;
period = 50;
numperiod = 1;
range = [20,100];
band = [0 1];
u = idinput([period,numchannel,numperiod],'rgs',band,range);
us = iddata([],u,100);
