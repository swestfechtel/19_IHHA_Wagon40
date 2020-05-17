allNames = parallel.clusterProfiles();
myCluster = parcluster('local');
myCluster.NumWorkers = 20;
saveProfile(myCluster);