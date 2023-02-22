#include "node.h"
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

typedef map<string, map<string, int>> MSMSI;
typedef map<string, map<string, NetInterface>> MSMSPSS;

void printRT(vector<RoutingNode*> nodes){
  /*Print routing table entries*/
	for (int i = 0; i < nodes.size(); i++) {
	  nodes[i]->printTable();
	}
}

MSMSI make_graph(MSMSI &edges_cost){
  MSMSI graph;
  for(auto &[node_snd, edge_cost]: edges_cost){
    for(auto &[node_rcv, cost]: edge_cost){
      graph[node_snd][node_rcv] = cost;
    }
  }
  return graph;
}

typedef map<string, int> MSI;
typedef map<string, string> MSS;

pair<MSI, MSS> dijkstra(string src, MSMSI &graph){
  map<string, int> dist;
  map<string, string> parent;
  set<pair<int, string>> heap;

  // initialize the dist vector with inf
  for(auto &[node, map]: graph){
    dist[node] = INT_MAX;
  }

  dist[src] = 0;
  heap.insert({0, src});
  parent[src] = src;

  while(!heap.empty()){
    auto [cost, node] = *heap.begin();
    heap.erase(heap.begin());

    for(auto [v, cost]: graph[node]){
      if(dist[node] + cost < dist[v]){
        heap.erase({dist[v], v});
        dist[v] = dist[node] + cost;
        heap.insert({dist[v], v});
        parent[v] = node;
      }
    }
  }
  
  return {dist, parent};
}

map<string, string> find_next_hops(string root, map<string, string>& parent){
  map<string, string> next_hops;
  for(auto [node, p]: parent){
    if(node == root){
      next_hops[node] = node;
    }else if(p == root){
      next_hops[node] = node;
    }else{
      while(p != root && parent[p] != root)
        p = parent[p];
      next_hops[node] = p;
    }
  }
  return next_hops;
}

void make_output(RoutingNode* src, vector<RoutingNode*>& nodes, MSMSPSS& edges_interfaces, map<string, int>& dist, map<string, string>& next_hops){
  routingtbl table;

  for(auto node: nodes){
    auto destination = node->getName();
    auto dest_next_hop = next_hops[destination];
    auto cost = dist[destination];

    for(auto ip: node->ips){
      auto dest_ip = ip;
      auto next_hop =  destination == src->getName() ? ip : edges_interfaces[src->getName()][dest_next_hop].getConnectedIp() ;
      auto ip_interface = destination == src->getName() ? ip : edges_interfaces[src->getName()][dest_next_hop].getip();
      table.tbl.push_back(RoutingEntry(dest_ip, next_hop, ip_interface, cost));
    }
  }

  src->setTable(table);
}

void routingAlgo(vector<RoutingNode*> nodes, MSMSI &edges_cost, MSMSPSS &edges_interfaces){
  // my code
  for(auto &node_snd: nodes){
    for(auto &node_rcv: nodes){
      if(node_snd != node_rcv)
        node_snd->sendMsg(node_rcv);
    }
  }

  for(auto node: nodes){
    auto graph = make_graph(edges_cost);


    auto [dist, parent] = dijkstra(node->getName(), graph);
    auto next_hops = find_next_hops(node->getName(), parent);

    make_output(node, nodes, edges_interfaces, dist, next_hops);
  }
 
  /*Print routing table entries after routing algo converges */
  printf("Printing the routing tables after the convergence \n");
  printRT(nodes);
}

void RoutingNode::recvMsg(RouteMsg *msg) {
  // Traverse the routing table in the message.
  // Check if entries present in the message table is closer than already present 
  // entries.
  // Update entries.
 
  routingtbl *recvRoutingTable = msg->mytbl;
  for (RoutingEntry entry : recvRoutingTable->tbl) {
    // Check routing entry
    bool entryExists = false;
    for(int i=0; i<mytbl.tbl.size(); ++i) {
      entryExists |= (mytbl.tbl[i].dstip == entry.dstip);
    }

    if(!entryExists) {
      RoutingEntry newEntry(entry.dstip, msg->from, msg->recvip, entry.cost+1);
      mytbl.tbl.push_back(newEntry);
    }
  }
}

