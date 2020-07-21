u = [20*ones(800*2,1); 27*ones(800*2,1); 15*ones(600*2,1); 10*ones(400*2,1); 22*ones(900*2,1); 0*ones(100*2,1)];
writematrix(u,'output/track.tsv','FileType','text','WriteMode','append','Delimiter','tab');
u = [10*ones(1000*2,1); 15*ones(500*2,1); 25*ones(1200*2,1); 12*ones(600*2,1); 0*ones(300*2,1)];
writematrix(u,'output/track2.tsv','FileType','text','WriteMode','append','Delimiter','tab');