#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <assert.h>
#include <fstream>
#include <cstring>
#include <sstream>

#include "net.h"

//********************************************************
//             Net Member Functions
//********************************************************

bool net::append_pin(pin *p) {
    connections_.push_back(p);
}

//********************************************************
//             Pin Member Functions
//********************************************************

bool pin::create(
    gate *g,
    size_t index,
    const evl_pin &p,
    const std::map<std::string, net *> &nets_table) {

    //store g and index
    //if(p.msb == -1) { //a 1-bit wire
    //  net_name = p.name;
    //  net_ = find net_name in nets_table
    //  net_->append_pin(pin);
    //}
}

//********************************************************
//             Gate Member Functions
//********************************************************

bool gate::create(
    const evl_component &c,
    const std::map<std::string, net *> &nets_table,
    const evl_wires_table &wires_table) {

    //set gate type and name
    //size_t index = 0;
    //for each evl_pin ep in c {
    //  create_pin(ep, index, nets_table, wires_table);
    //  index++
    //}
    //return validate_structural_semantics()
    return true;
}

bool gate::create_pin(
    const evl_pin &ep,
    size_t index,
    const std::map<std::string, net *> &nets_table,
    const evl_wires_table &wires_table) {

    //resolve semantics of ep using wires_table
    //pin *p = new pin;
    //pins_.push_back(p);
    //return p->create(this, index, ep, nets_table)
    return true;
}

//********************************************************
//             Netlist Member Functions
//********************************************************

bool netlist::create_net(std::string net_name) {
    assert(nets_table_.find(net_name) == nets_table_.end());
    net *n = new net(net_name);
    nets_table_[net_name] = n;
    nets_.push_back(n);
}

std::string netlist::make_net_name(std::string wire_name, int i) {
    assert(i>= 0);
    std::ostringstream oss;
    oss << wire_name << "[" << i << "]";
    return oss.str();
}

bool netlist::create_nets(const evl_wires &wires) {
    for(evl_wires::const_iterator wire = wires.begin(); wire != wires.end(); ++wire) {
        if(wire.width == 1) {
            create_net(wire.name);
        } else {
            for(int i = 0; i < wire.width; ++i) {
                create_net(make_net_name(wire.name, i));
            }
        }
    }
    return true;
}

bool netlist::create_gate(
    const evl_component &c,
    const evl_wires_table &wires_table) {

    gate *g = new gate;
    gates_.push_back(g);
    return g->create(c, nets_table_, wires_table);
}

bool netlist::create_gates(
    const evl_components &comps,
    const evl_wires_table &wires_table) {

    for(evl_components::const_iterator c = comps.begin(); c != comps.end(); ++c) {
        create_gate(c, wires_table);
    }
    return true;
}

bool netlist::create(
    const evl_wires &wires,
    const evl_components &comps,
    const evl_wires_table &wires_table) {

    return create_nets()
        && create_gates();
}

void netlist::display(
    std::ostream &out) const {

    std::cout << "Display Not Implemented" << std::endl;
}

bool netlist::store(
    std::string file_name) const {

    std::ofstream output_file(file_name.c_str());

    if(!output_file) {
        std::cerr << "I can't write " << file_name << std::endl;
        return -1;
    }

    display(output_file);
    return true;
}
