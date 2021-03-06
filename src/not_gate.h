#ifndef NOTGATE
#define NOTGATE

#include <string>
#include <list>
#include <vector>
#include <assert.h>

#include "net.h"

class not_gate: public gate{
public:
    not_gate(std::string name)
        : gate("not", name) {}

    bool validate_structural_semantics() override;
    void compute_state_or_output(std::string file_name) override;
    char compute_signal(int pin_index) override;
}; // class buffer gate

bool not_gate::validate_structural_semantics() {
    if (pins_.size() != 2)
        return false;
    pins_[0]->set_as_output(); // o
    pins_[1]->set_as_input(); // i
    return true;
}

void not_gate::compute_state_or_output(std::string file_name) {
}

char not_gate::compute_signal(int pin_index) {
    assert(pin_index == 0); // must be q
    return not pins_[1]->compute_signal();
}

#endif
