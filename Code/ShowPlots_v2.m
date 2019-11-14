close all;

figure;
subplot(221);
plot(force.Time, 1/1000*force.Data);
xlabel('t [s]');
ylabel('F_B [kN]');

subplot(222);
plot(pressure.Time, 5+pressure.Data);
ylabel('p_{BP} [bar]');
xlabel('t [s]');
ylim([0 5.5]);
xlim([0 tmax]);

subplot(223);
plot(acceleration_pos.Time, acceleration_pos.Data);
xlabel('t [s]');
ylabel('a [{m \over s^{2}}]');

subplot(224);
plot(acceleration_neg.Time, acceleration_neg.Data);
xlabel('t [s]');
ylabel('a_{res} [{m \over s^{2}}]');