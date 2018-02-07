import networkx as nx
from ksp import k_shortest_paths
# import matplotlib.pyplot as plt
filepath = '../data/input_files_PGH/'
filepath = '../data/input_files_2link/'
G = nx.Graph()

with open(filepath+'MNM_input_node','r') as f:
	f.next()
	for line in f:
		items = line.strip().split(" ")
		nodeID = int(items[0])
		nodeType = items[1]
		G.add_node(nodeID)

with open(filepath+'Snap_graph','r') as f:
	f.next()
	for line in f:

		items = line.strip().split(" ")
		lineID = int(items[0])
		inNode = int(items[1])
		outNode = int(items[2])
		G.add_edge(inNode,outNode,label = lineID)


with open(filepath+'MNM_input_link','r') as f:
	f.next()
	for line in f:
		items = line.strip().split(" ")
		nx.set_edge_attributes(G,'weight',float(items[2])/float(items[3]))


odict = {}
ddict = {}
oNow = 1
odpair = []

with open(filepath+"MNM_input_od",'r') as f:
	f.next()

	for line in f:
		items = line.strip().split(" ")
		if len(items) !=2:
			oNow = 0
			continue
		elif oNow==1:
			odict[int(items[0])] = int(items[1])
		else:
			ddict[int(items[0])] = int(items[1])




with open(filepath+'MNM_input_demand','r') as f:
	f.next()
	for line in f:
		# print line
		items = line.strip().split(" ")
		if len(items) < 2:
			break
		# print "length",len(items)
		# print items[0],items[1]
		odpair.append((int(items[0]),int(items[1])))
count = 1
for od in odpair:
	o = odict[od[0]]
	d = ddict[od[1]]
	# a =  nx.all_simple_paths(G,o,d)
	ksps = k_shortest_paths(G,o,d,k=2)
	path = str(o) +"," + str(d) + "#"
	# for e in a:
		
	# 	s = e[0]
	# 	for ed in e[1:]:
	# 		path += str(G.get_edge_data(s,ed)['label']) + "," 
	# 		s = ed
	# 	path = path[:-1]+"#"
	for i in range(len(ksps[1])):
		thispath = ksps[1][i]
		for j in range(len(thispath)-1):


			path += str(G.get_edge_data(thispath[j],thispath[j+1])['label'])+ ","
		# print str(G.get_edge_data(ksps[1][i],ksps[1][i+1])['label'])/
		path = path[:-1]+"#"

	path = path[:-1]
	print path
	count +=1 
	if count>10:
		break
	# print ksps[1]
	# print path,ksps[1]

	# break

# Plot
# pos = nx.spring_layout(G)
# nx.draw_networkx_nodes(G, pos, cmap=plt.get_cmap('jet'), 
#      node_size = 500)
# nx.draw_networkx_labels(G, pos)
# nx.draw_networkx_edges(G, pos)
# nx.draw_networkx_edge_labels(G, pos)
# plt.show()


		



