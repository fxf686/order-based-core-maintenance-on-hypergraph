/*
[File]
order.cpp is  the OCM mothod, and hybrid.cpp is the OTCM method.
mcd.cpp is the mcd mothod, pcd.cpp is the PCD method, mcd_main.cpp is our modified MCD method.
HypergraphCoreDecomp is the core decompostion method.


[To compile]
g++ -std=cv++11 -O3 file.cpp GraphScheduler.cpp Hypergraph.cpp HypergraphCoreDecomp.cpp  -o file

[To run]
file filename

[Example]
g++ -std=cv++11 -O3 filename.cpp GraphScheduler.cpp Hypergraph.cpp HypergraphCoreDecomp.cpp  -o hybrid

order ./s2.txt

[Format of input]
The file should contain an update in each line.
Hyperedge insertion: + [node IDs] timestamp
Hyperedge deletion: - [node IDs]
So be careful that the last number of a line beginning with "+" is not an endpoint of the inserted hyperedge.
All the dataset downloaded should be preprocessed into this format  for further analysis.
*/