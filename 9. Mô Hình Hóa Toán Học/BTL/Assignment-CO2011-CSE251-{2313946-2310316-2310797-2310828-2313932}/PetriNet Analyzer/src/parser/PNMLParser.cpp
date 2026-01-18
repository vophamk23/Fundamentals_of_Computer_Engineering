#include "PNMLParser.h"
#include "../include/tinyxml2.h"
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace tinyxml2;

// Parse file PNML (Petri Net Markup Language) thành đối tượng PetriNet
PetriNet PNMLParser::parse(const string &filename)
{
    // Đọc và load file XML
    XMLDocument doc;
    XMLError result = doc.LoadFile(filename.c_str());

    if (result != XML_SUCCESS)
    {
        throw runtime_error("Cannot load PNML file: " + filename);
    }

    // Điều hướng cấu trúc XML: pnml -> net -> page
    XMLElement *pnml = doc.FirstChildElement("pnml");
    XMLElement *netElem = pnml->FirstChildElement("net");
    XMLElement *page = netElem->FirstChildElement("page");

    if (!pnml || !netElem || !page)
    {
        throw runtime_error("Invalid PNML structure");
    }

    // Tạo đối tượng PetriNet
    PetriNet petri_net;

    // Parse Places
    for (XMLElement *elem = page->FirstChildElement("place");
         elem; elem = elem->NextSiblingElement("place"))
    {
        const char *id = elem->Attribute("id");
        if (!id)
            continue;

        string name = get_text(elem, "name");
        int tokens = get_int(elem, "initialMarking", 0);

        petri_net.add_place(Place(id, name, tokens));
    }

    // Parse Transitions
    for (XMLElement *elem = page->FirstChildElement("transition");
         elem; elem = elem->NextSiblingElement("transition"))
    {
        const char *id = elem->Attribute("id");
        if (!id)
            continue;

        string name = get_text(elem, "name");
        petri_net.add_transition(Transition(id, name));
    }

    // Parse Arcs
    for (XMLElement *elem = page->FirstChildElement("arc");
         elem; elem = elem->NextSiblingElement("arc"))
    {
        const char *id = elem->Attribute("id");
        const char *src = elem->Attribute("source");
        const char *tgt = elem->Attribute("target");

        if (!id || !src || !tgt)
            continue;

        int weight = get_int(elem, "inscription", 1);
        petri_net.add_arc(Arc(id, src, tgt, weight));
    }

    return petri_net;
}

// Lấy text từ tag con theo cấu trúc PNML: <tag><text>value</text></tag>
string PNMLParser::get_text(void *element, const char *tag)
{
    XMLElement *elem = static_cast<XMLElement *>(element);
    XMLElement *child = elem->FirstChildElement(tag);
    if (!child)
        return "";

    XMLElement *text = child->FirstChildElement("text");
    if (!text || !text->GetText())
        return "";

    return text->GetText();
}

// Lấy số nguyên từ tag con
int PNMLParser::get_int(void *element, const char *tag, int default_val)
{
    string text = get_text(element, tag);
    if (text.empty())
        return default_val;

    try
    {
        return stoi(text);
    }
    catch (...)
    {
        return default_val;
    }
}