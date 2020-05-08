function ret = Write(id,velocity,force,pressure,distance,acceleration,wagon_ids,t,u,grad,ft,fc)
	numrows = get(force,'Length');
	time = force.Time;
	% mkdir('output/');
	matrix = [];

	for i = 1:1:numrows
		f = getdatasamples(force,i);
		p = getdatasamples(pressure,i);
		v = getdatasamples(velocity,i);
		a = getdatasamples(acceleration,i);
		% d = getdatasamples(distance,i);
		d = 0;
		row = [id,time(i),f,p,v,a,d,wagon_ids,grad,ft,fc]; 	% TODO: add simulation input aka track profile to matrix
															%		print wagon ids as list delimited by colon (,)
															%		performance?
		matrix = [matrix;row];
	end
	% writematrix(matrix,'output/output.tsv','FileType','text','WriteMode','append','Delimiter','tab');
	% ret = 1;
	fprintf('Create output matrix for Simulation ID %d\n', id);
	ret = matrix;
end