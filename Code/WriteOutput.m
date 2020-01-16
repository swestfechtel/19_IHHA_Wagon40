function ret = WriteOutput(name,velocity,force,pressure,distance,acceleration_neg,ids,t,u,trackgradient,Ft,fc)
    V = [velocity.Time, velocity.Data];
    F = [force.Time, force.Data];
    P = [pressure.Time, pressure.Data];
    D = [distance.Time, distance.Data];
    A = [acceleration_neg.Time, acceleration_neg.Data];
    t = transpose(t);
    M = [trackgradient, Ft, fc];
    output = [t, u];
    mkdir(strcat('output/',name));
    dir = strcat('output/',name);
    writematrix(V,strcat(dir,'/Velocity.csv'));
    writematrix(F,strcat(dir,'/Force.csv'));
    writematrix(P,strcat(dir,'/Pressure.csv'));
    writematrix(D,strcat(dir,'/Distance.csv'));
    writematrix(A,strcat(dir,'/Acceleration_neg.csv'));
    writematrix(ids,strcat(dir,'/ids.csv'));
    writematrix(output,strcat(dir,'/input.csv'));
    writematrix(M,strcat(dir,'/meta.csv'));
    ret = 1;
end