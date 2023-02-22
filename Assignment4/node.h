#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

/*
  Each row in the table will have these fields
  dstip:	Destination IP address
  nexthop: 	Next hop on the path to reach dstip
  ip_interface: nexthop is reachable via this interface (a node can have multiple interfaces)
  cost: 	cost of reaching dstip (number of hops)
*/
class RoutingEntry{
 public:
  string dstip, nexthop, ip_interface;
  int cost;

  RoutingEntry() {}

  RoutingEntry(string dstip, string nexthop, string ip_interface, int cost) 
    : dstip(dstip), nexthop(nexthop), ip_interface(ip_interface), cost(cost) {}

  // Class for specifying the sort order of Routing Table Entries
  // while printing the routing tables
  bool operator<(const RoutingEntry &R2){
    if (cost == R2.cost) 
      return dstip.compare(R2.dstip)<0;
    else if(cost > R2.cost)
      return false;
    else
      return true;
  }
};

/*
  This is the routing table
*/
class routingtbl {
public:
  vector<RoutingEntry> tbl;
};

/*
  Message format to be sent by a sender
  from: 		Sender's ip
  mytbl: 		Senders routing table
  recvip:		Receiver's ip
*/
class RouteMsg {
 public:
  string from;			// I am sending this message, so it must be me i.e. if A is sending mesg to B then it is A's ip address
  struct routingtbl *mytbl;	// This is routing table of A
  string recvip;		// B ip address that will receive this message

  RouteMsg() {}

  RouteMsg(string from, routingtbl* mytbl, string recvip)
    : from(from), mytbl(mytbl), recvip(recvip) {}
};

/*
  Emulation of network interface. Since we do not have a wire class, 
  we are showing the connection by the pair of IP's
  
  ip: 		Own ip
  connectedTo: 	An address to which above mentioned ip is connected via ethernet.
*/
class NetInterface {
 private:
  string ip;
  string connectedTo; 	//this node is connected to this ip
  
 public:
  NetInterface() {}

  NetInterface(string ip, string connectedTo) : ip(ip), connectedTo(connectedTo) {}

  string getip() {
    return this->ip;
  }
  string getConnectedIp() {
    return this->connectedTo;
  }
  void setip(string ip) {
    this->ip = ip;
  }
  void setConnectedip(string ip) {
    this->connectedTo = ip;
  }
};

/*
  Struct of each node
  name: 	It is just a label for a node
  interfaces: 	List of network interfaces a node have
  Node* is part of each interface, it easily allows to send message to another node
  mytbl: 		Node's routing table
*/
class Node {
 private:
  string name;
  
 protected:
  struct routingtbl mytbl;
  
  virtual void recvMsg(RouteMsg* msg) {
    cout<<"Base"<<endl;
  }

  bool isMyInterface(string eth) {
    for (int i = 0; i < interfaces.size(); ++i) {
      if(interfaces[i].first.getip() == eth)
        return true;
    }
    return false;
  }

 public:
  vector<pair<NetInterface, Node*>> interfaces;
  vector<string> ips;

  void addInterface(string ip, string connip, Node *nextHop) {
    NetInterface eth;
    eth.setip(ip);
    eth.setConnectedip(connip);
    interfaces.push_back({eth, nextHop});
  }

  void updateTblEntry(string dstip, string nexthop, string ip_interface, int cost) {
    // to update the dstip hop count in the routing table (if dstip already exists)
    // new hop count will be equal to the cost 
    bool found = false;
    for(int i=0; i<mytbl.tbl.size(); i++){
      if(mytbl.tbl[i].dstip == dstip) {
        mytbl.tbl[i] = RoutingEntry(dstip, nexthop, ip_interface, cost);
        found = true;
      }
    }

    if(!found){
      mytbl.tbl.push_back(RoutingEntry(dstip, nexthop, ip_interface, cost));
    }
  }
  
  string getName() {
    return this->name;
  }

  void setName(string name){
    this->name = name;
  }
  
  void setTable(routingtbl tbl){
    mytbl = tbl;
  }

  struct routingtbl getTable() {
    return mytbl;
  }
  
  void printTable() {
    sort(mytbl.tbl.begin(),mytbl.tbl.end());
    cout << getName() << ":" << endl;
    for(int i=0; i<mytbl.tbl.size(); ++i) {
      cout << mytbl.tbl[i].dstip << " | " << mytbl.tbl[i].nexthop << " | " 
        << mytbl.tbl[i].ip_interface << " | " << mytbl.tbl[i].cost << endl;
    }
  }
  
  void sendMsg(Node* node_rcv){
    struct routingtbl ntbl;
    for (int i = 0; i < mytbl.tbl.size(); ++i) {
      ntbl.tbl.push_back(mytbl.tbl[i]);
    }
    
    RouteMsg msg(ips[0], &ntbl, node_rcv->ips[0]);
    node_rcv->recvMsg(&msg);
  }

};

class RoutingNode: public Node {
 public:
  void recvMsg(RouteMsg *msg);
};
