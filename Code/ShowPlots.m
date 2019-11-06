close all
tplot = force.Time;
Nwagon = 40;

step = 6;
legendstring = [];
for i = 2:step:Nwagon;
    legendstring = [legendstring; i];
end
legendstring = num2str(legendstring);


figure
subplot(321)
plot(tplot,1/1000*force.Data(:,1:step:Nwagon));
ylabel('F_B/kN')
%xlabel('t/s')
%legend(legendstring)
subplot(323)
plot(tplot, 1/1000*diff(force.Data(:,1:step:Nwagon-1),1,2))
ylabel('\Delta F/kN')
%xlabel('t/s')
%legend(legendstring)
subplot(325)
plot(tplot,5+pressure.Data(:,1:step:Nwagon));
ylabel('p_{BP}/bar')
xlabel('t/s')
legend(legendstring)
ylim([0 5.5])
xlim([0 tmax])

subplot(322)
plot(tplot,1/1000*force1.Data(:,1:step:Nwagon));
ylabel('F_B/kN')
%xlabel('t/s')
%legend(legendstring)
subplot(324)
plot(tplot, 1/1000*diff(force1.Data(:,1:step:Nwagon-1),1,2))
ylabel('\Delta F/kN')
%xlabel('t/s')
%legend(legendstring)
subplot(326)
plot(tplot,5+pressure1.Data(:,1:step:Nwagon));
ylabel('p_{BP}/bar')
xlabel('t/s')
legend(legendstring)
ylim([0 5.5])
xlim([0 tmax])

figure
subplot(2,1,1)
plot(tplot,velocity.Data(:,1:step:Nwagon));
ylabel('v/m/s')
xlabel('t/s')
legend(legendstring)

subplot(2,1,2)
plot(tplot,velocity1.Data(:,1:step:Nwagon));
ylabel('v/m/s')
xlabel('t/s')
legend(legendstring)