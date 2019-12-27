close all;

figure;
subplot(221);
plot(force.Time, 1/1000*force.Data(:,1:5:40));
xlabel('t [s]');
ylabel('F_B [kN]');

subplot(222);
plot(pressure.Time, 5+pressure.Data(:,1:5:40));
ylabel('p_{BP} [bar]');
xlabel('t [s]');
ylim([0 5.5]);
xlim([0 tmax]);

subplot(223);
plot(velocity.Time, velocity.Data);
xlabel('t [s]');
ylabel('v [{m \over s}]');

subplot(224);
plot(acceleration_neg.Time, acceleration_neg.Data(:,1:5:40));
xlabel('t [s]');
ylabel('a_{res} [{m \over s^{2}}]');

figure;
subplot(211);
plot(distance);

subplot(212);
plot(u);
