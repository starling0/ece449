#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <assert.h>
#include <fstream>
#include <cstring>
#include <sstream>
#include <algorithm>

#include "net.h"
#include "syn.h"

//********************************************************
//             Net Member Functions
//********************************************************

void net::append_pin(pin *p) {
    connections_.push_back(p);
}

// This function can be improved
bool net::validate_connections() {
    size_t counter = 0;
    for_each(connections_.begin(), connections_.end(),
        [&] (pin *p) {
            if(p->get_dir() == 'O') {
                counter++;
            }
        }
    );
    if(counter != 1) {
        std::cerr << "Error: signal wire '" << name_
            << "' has multiple outputs" << std::endl;
        return false;
    }
    return true;
}

void net::display(
    std::ostream &out) const {
    out << "  net " << name_ << " " << connections_.size() << std::endl;
    for_each(connections_.begin(), connections_.end(),
        [&] (pin *p) {
            out << "    " << p->get_gate()->get_type();
            if(p->get_gate()->get_name() != "") {
                out << " "  << p->get_gate()->get_name();
            }
            out << " " << p->get_index() << std::endl;
        }
    );
}

//********************************************************
//             Pin Member Functions
//********************************************************

bool pin::create(
    gate *g,
    size_t index,
    const evl_pin &p,
    const std::map<std::string, net *> &nets_table) {

    //std::cout << "Creating " << p.get_name() << " pin" << std::endl;
    gate_ = g;
    index_ = index;
    if(p.get_msb() == -1) {
        //std::cout << "in msb = -1" << std::endl;
        if(p.get_lsb() == -1) {
            //std::cout << "  in lsb = -1" << std::endl;
            if(nets_table.find(p.get_name()) != nets_table.end()) {
                //std::cout << "  is a single wire" << std::endl;
                net *net_ptr = nets_table.find(p.get_name())->second;
                net_ptr->append_pin(this);
                nets_.push_back(net_ptr);
            } else if(nets_table.find(p.get_name() + "[0]") != nets_table.end()) {
                //std::cout << "  is a bus" << std::endl;
                size_t i = 0;
                std::ostringstream temp;
                temp << p.get_name() << "[" << i << "]";
                for(; nets_table.find(temp.str()) != nets_table.end();) {
                    //std::cout << "connecting index " << i << std::endl;
                    std::ostringstream oss;
                    oss << p.get_name() << "[" << i << "]";
                    net *net_ptr = nets_table.find(oss.str())->second;
                    net_ptr->append_pin(this);
                    nets_.push_back(net_ptr);
                    //std::cout << "index " << i << " connected" << std::endl;
                    ++i;
                    temp.str("");
                    temp << p.get_name() << "[" << i << "]";
                }
            } else {
                std::cerr << "'" << p.get_name()
                    << "' was not found in nets_table" << std::endl;
                return false;
            }
        } else {
            std::cerr << "something went wrong ..." << std::endl;
            return false;
        }
    } else if(p.get_msb() >= 0) {
        //std::cout << "in msb != -1" << std::endl;
        if(p.get_lsb() == -1) {
            //std::cout << "  in lsb = -1" << std::endl;
            std::ostringstream oss;
            oss << p.get_name() << "[" << p.get_msb() << "]";
            net *net_ptr = nets_table.find(oss.str())->second;
            net_ptr->append_pin(this);
            nets_.push_back(net_ptr);
        } else if((p.get_lsb() >= 0) && (p.get_lsb() <= p.get_msb())) {
            //std::cout << "in lsb != -1" << std::endl;
            for(int i = p.get_lsb(); i <= p.get_msb(); ++i) {
                std::ostringstream oss;
                oss << p.get_name() << "[" << i << "]";
                net *net_ptr = nets_table.find(oss.str())->second;
                net_ptr->append_pin(this);
                nets_.push_back(net_ptr);
            }
        } else {
            std::cerr << "incorrect lsb pin assignment" << std::endl;
            return false;
        }
    } else {
        std::cerr << "incorrect msb pin assignment" << std::endl;
        return false;
    }

    //std::cout << "Pin created" << std::endl;
    return true;
}

void pin::display(
    std::ostream &out) const {

    if(!nets_.empty()) {
        out << "    pin " << nets_.size();
        for_each(nets_.begin(), nets_.end(),
            [&] (net *n) {
                out << " " << n->get_name();
            }
        );
        out << std::endl;
    }
}

//********************************************************
//             Gate Member Functions
//********************************************************

bool gate::validate_structural_semantics() {
    if((type_ == "and") || (type_ == "or") || (type_ == "xor")) {
        if(pins_.size() < 3) {
            std::cerr << "[" << type_ << "] " << name_
                << ": invalid number of inputs" << std::endl;
            return false;
        }
        pins_.front()->set_dir('O');
        for_each(std::next(pins_.begin()), pins_.end(),
            [&] (pin *p) {
                p->set_dir('I');
            }
        );
    } else if((type_ == "not") || (type_ == "buf")) {
        if(pins_.size() != 2) {
            std::cerr << "[" << type_ << "] " << name_
                << ": invalid number of inputs" << std::endl;
            return false;
        }
        pins_.front()->set_dir('O');
        for_each(std::next(pins_.begin()), pins_.end(),
            [&] (pin *p) {
                p->set_dir('I');
            }
        );
    } else if((type_ == "evl_dff") || (type_ == "tris")) {
        if(pins_.size() != 3) {
            std::cerr << "[" << type_ << "] " << name_
                << ": invalid number of inputs" << std::endl;
            return false;
        }
        pins_.front()->set_dir('O');
        for_each(std::next(pins_.begin()), pins_.end(),
            [&] (pin *p) {
                p->set_dir('I');
            }
        );
    } else if((type_ == "evl_clock")) {
        if(pins_.size() != 1) {
            std::cerr << "[" << type_ << "] " << name_
                << ": invalid number of inputs" << std::endl;
            return false;
        }
        pins_.front()->set_dir('O');
    } else if((type_ == "evl_one") || (type_ == "evl_zero") || (type_ == "evl_input")) {
        if(pins_.empty()) {
            std::cerr << "[" << type_ << "] Warning: no input signals specified"
                << std::endl;
        }
        for_each(pins_.begin(), pins_.end(),
            [&] (pin *p) {
                p->set_dir('O');
            }
        );
    } else if((type_ == "evl_output")) {
        if(pins_.empty()) {
            std::cerr << "[" << type_ << "] Warning: no output signals specified"
                << std::endl;
        }
        for_each(std::next(pins_.begin()), pins_.end(),
            [&] (pin *p) {
                p->set_dir('I');
            }
        );
    } else {
        std::cerr << "Warning: type '" << type_
            << "' does not exist" << std::endl;
        //return false;
    }
    return true;
}

bool gate::create(
    const evl_component &c,
    const std::map<std::string, net *> &nets_table,
    const evl_module::evl_wires_table &wires_table) {

    name_ = c.get_name();
    type_ = c.get_type();

    size_t index = 0;
    for(evl_pins::const_iterator pin_ = c.pins_begin();
        pin_ != c.pins_end(); ++pin_) {

        pin *p = new pin;
        pins_.push_back(p);
        if(!(p->create(this, index, *pin_, nets_table))) {
            return false;
        }
        index++;
    }
    return true;
    //return validate_structural_semantics();
}

void gate::display(
    std::ostream &out) const {

    if(!pins_.empty()) {
        out << "  component " << type_;
        if(name_ != "") {
            out << " " << name_;
        }
        out << " " << pins_.size() << std::endl;
        for_each(pins_.begin(), pins_.end(),
            [&] (pin *p) {
                p->display(out);
            }
        );
    }
}

//********************************************************
//             Netlist Member Functions
//********************************************************

void netlist::create_net(std::string net_name) {
    assert(nets_table_.find(net_name) == nets_table_.end());
    net *n = new net(net_name);
    nets_table_[net_name] = n;
    nets_.push_back(n);
}

bool netlist::create(
    const evl_module &module) {

    name_ = module.get_name();

    for(evl_wires::const_iterator wire = module.wires_begin();
        wire != module.wires_end(); ++wire) {

        if(wire->get_width() == 1) {
            create_net(wire->get_name());
        } else {
            for(int i = 0; i < wire->get_width(); ++i) {
                std::ostringstream oss;
                oss << wire->get_name() << "[" << i << "]";
                create_net(oss.str());
            }
        }
    }
    for(evl_components::const_iterator c = module.components_begin();
        c != module.components_end(); ++c) {

        gate *g = new gate;
        gates_.push_back(g);
        if(!(g->create(*c, nets_table_, module.get_wires_table())))
            return false;
    }
    for_each(nets_.begin(), nets_.end(),
        [&] (net *n) {
            if(!(n->validate_connections())) {
                return false;
            }
            return true;
        }
    );

    if(gates_.empty()) {
        if(nets_.empty()) {
            std::cout << "[Module " << name_
                << "] Warning: no gates declared" << std::endl;
        } else {
            std::cout << "[Module " << name_
                << "] Warning: empty netlist" << std::endl;
        }
    }
    return true;
}

void netlist::display(
    std::ostream &out) const {

    out << "module " << name_ << std::endl;
    if(!nets_.empty()) {
        out <<  "nets " << nets_.size() << std::endl;
        for_each(nets_.begin(), nets_.end(),
            [&] (net *n) {
                n->display(out);
            }
        );
    }
    if(!gates_.empty()) {
        out << "components " << gates_.size() << std::endl;
        for_each(gates_.begin(), gates_.end(),
            [&] (gate *g) {
                g->display(out);
            }
        );
    }
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
