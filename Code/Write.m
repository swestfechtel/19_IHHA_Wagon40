function ret = Write(id,velocity,force,pressure,distance,acceleration,wagon_ids,t,u,grad,ft,fc)
	numrows = get(force,'Length');
	cntwagon_ids = wagon_ids(wagon_ids~=0);
	wagon_ids = double(wagon_ids);
	numwagons = length(cntwagon_ids);
	time = force.Time;
	filler = zeros(1,40 - numwagons);
	matrix = [];

	for i = 1:1:numrows
		f = getdatasamples(force,i);
		% f = [f,filler];
		p = getdatasamples(pressure,i);
		% p = [p,filler];
		v = getdatasamples(velocity,i);
		% v = [v,filler];
		a = getdatasamples(acceleration,i);
		% a = [a,filler];
		d = getdatasamples(distance,i);
		% fprintf('Dimensions:\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n',length(id),length(time(i)),length(f),length(p),length(v),length(a),length(d),length(wagon_ids),length(grad),length(ft),length(fc));
		row = [id,time(i),f,p,v,a,d,wagon_ids,grad,ft,fc]; 	% TODO: add simulation input aka track profile to matrix
															%		print wagon ids as list delimited by colon (,)
															%		performance?
		matrix = [matrix;row];
	end
	% writematrix(matrix,'output/output.tsv','FileType','text','WriteMode','append','Delimiter','tab');
	% ret = 1;
	% fprintf('Create output matrix for Simulation ID %d\n', id);
	ret = matrix;
end