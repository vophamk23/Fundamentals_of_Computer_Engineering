#include "PetriNet.h"
#include <iostream>
#include <algorithm>

using namespace std;

// ========== Place ==========
Place::Place(const string &id, const string &name, int tokens)
    : id(id), name(name), initial_tokens(tokens) {}

void Place::print() const
{
    cout << "     ○ Place[" << id << "]";
    if (!name.empty())
        cout << " \"" << name << "\"";
    cout << " [tokens=" << initial_tokens << "]\n";
}

// ========== Transition ==========
Transition::Transition(const string &id, const string &name)
    : id(id), name(name) {}

void Transition::add_input(const string &place_id)
{
    input_places.push_back(place_id);
}

void Transition::add_output(const string &place_id)
{
    output_places.push_back(place_id);
}

void Transition::print() const
{
    cout << "     ▭ Transition[" << id << "]";
    if (!name.empty())
        cout << " \"" << name << "\"\n";
    cout << "        Inputs: ";
    for (const auto &p : input_places)
        cout << p << " ";
    cout << "\n        Outputs: ";
    for (const auto &p : output_places)
        cout << p << " ";
    cout << "\n";
}

// ========== Arc ==========
Arc::Arc(const string &id, const string &src, const string &tgt, int w)
    : id(id), source(src), target(tgt), weight(w) {}

// ========== Petri Net ==========
void PetriNet::add_place(const Place &p)
{
    places[p.id] = p;
    initial_marking[p.id] = p.initial_tokens;
    place_order.push_back(p.id);
}

void PetriNet::add_transition(const Transition &t)
{
    transitions[t.id] = t;
    transition_order.push_back(t.id);
}

void PetriNet::add_arc(const Arc &a)
{
    arcs.push_back(a);

    if (places.count(a.source) && transitions.count(a.target))
    {
        transitions[a.target].add_input(a.source);
    }
    else if (transitions.count(a.source) && places.count(a.target))
    {
        transitions[a.source].add_output(a.target);
    }
}

Transition *PetriNet::get_transition(const string &id)
{
    auto it = transitions.find(id);
    return (it != transitions.end()) ? &(it->second) : nullptr;
}

// In theo format ma trận - ĐÂY LÀ HÀM DUY NHẤT XUẤT OUTPUT
void PetriNet::print() const
{
    // 1. In danh sách Places
    cout << "Places: [";
    for (size_t i = 0; i < place_order.size(); i++)
    {
        cout << "'" << place_order[i] << "'";
        if (i < place_order.size() - 1)
            cout << ", ";
    }
    cout << "]\n";

    // 2. In tên các Places
    cout << "Place names: [";
    for (size_t i = 0; i < place_order.size(); i++)
    {
        const Place &p = places.at(place_order[i]);
        cout << "'" << p.name << "'";
        if (i < place_order.size() - 1)
            cout << ", ";
    }
    cout << "]\n\n";

    // 3. In danh sách Transitions
    cout << "Transitions: [";
    for (size_t i = 0; i < transition_order.size(); i++)
    {
        cout << "'" << transition_order[i] << "'";
        if (i < transition_order.size() - 1)
            cout << ", ";
    }
    cout << "]\n";

    // 4. In tên các Transitions
    cout << "Transition names: [";
    for (size_t i = 0; i < transition_order.size(); i++)
    {
        const Transition &t = transitions.at(transition_order[i]);
        cout << "'" << t.name << "'";
        if (i < transition_order.size() - 1)
            cout << ", ";
    }
    cout << "]\n\n";

    // 5. Ma trận I (Input matrix)
    cout << "I (input) matrix:\n";
    cout << "[";
    for (size_t t = 0; t < transition_order.size(); t++)
    {
        const Transition &trans = transitions.at(transition_order[t]);
        cout << "[";
        for (size_t p = 0; p < place_order.size(); p++)
        {
            const string &place_id = place_order[p];

            bool is_input = find(trans.input_places.begin(),
                                 trans.input_places.end(),
                                 place_id) != trans.input_places.end();

            cout << (is_input ? 1 : 0);
            if (p < place_order.size() - 1)
                cout << " ";
        }
        cout << "]";
        if (t < transition_order.size() - 1)
            cout << "\n ";
    }
    cout << "]\n\n";

    // 6. Ma trận O (Output matrix)
    cout << "O (output) matrix:\n";
    cout << "[";
    for (size_t t = 0; t < transition_order.size(); t++)
    {
        const Transition &trans = transitions.at(transition_order[t]);
        cout << "[";
        for (size_t p = 0; p < place_order.size(); p++)
        {
            const string &place_id = place_order[p];

            bool is_output = find(trans.output_places.begin(),
                                  trans.output_places.end(),
                                  place_id) != trans.output_places.end();

            cout << (is_output ? 1 : 0);
            if (p < place_order.size() - 1)
                cout << " ";
        }
        cout << "]";
        if (t < transition_order.size() - 1)
            cout << "\n ";
    }
    cout << "]\n\n";

    // 7. Initial marking M0
    cout << "Initial marking M0: [";
    for (size_t p = 0; p < place_order.size(); p++)
    {
        cout << initial_marking.at(place_order[p]);
        if (p < place_order.size() - 1)
            cout << " ";
    }
    cout << "]\n";
}