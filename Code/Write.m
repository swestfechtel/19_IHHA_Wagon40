function ret = Write(id,velocity,force,pressure,distance,acceleration,wagon_ids,u,grad,ft,fc)
	numrows = get(force,'Length');
	wagon_ids = double(wagon_ids);
	time = force.Time;
	matrix = [];

	for i = 1:1:numrows
		f = getdatasamples(force,i);
		p = getdatasamples(pressure,i);
		v = getdatasamples(velocity,i);
		a = getdatasamples(acceleration,i);
		d = getdatasamples(distance,i);
		
		row = [id,time(i),f,p,v,a,d,wagon_ids,grad,ft,fc];
		matrix = [matrix;row];
    end
    
    %for i = numrows:1:length(u)
    %   f = zeros(1,40);
	%	p = zeros(1,40);
	%	v = 0;
	%	a = zeros(1,40);
	%	d = 0;
        
    %    row = [id,0,f,p,v,a,d,wagon_ids,grad,ft,fc,u(i)];
    %    matrix = [matrix;row];
    %end
    
	fprintf('Created output matrix for simid %d\n',id);
	% writematrix(matrix,'output/output.tsv','FileType','text','WriteMode','append','Delimiter','tab');
	% ret = 1;
	% fprintf('Create output matrix for Simulation ID %d\n', id);
	ret = matrix;
end