#include "xml_parsed.h"

#include <iostream>

void printElementNames(xmlNode *node) {
    for (xmlNode *cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            std::cout << "Node Name: " << cur_node->name << std::endl;

            // Print attributes if any
            for (xmlAttr *attr = cur_node->properties; attr; attr = attr->next) {
                xmlChar *value = xmlNodeGetContent((xmlNode *)attr);
                std::cout << "  Attribute: " << attr->name << " = " << value << std::endl;
                xmlFree(value);
            }
        }

        // Recursive call for child nodes
        printElementNames(cur_node->children);
    }
}

int main() {
    const char *filename = "files/simple.xml";
    xmlDoc *document = xmlReadFile(filename, nullptr, 0);

    if (!document) {
        std::cerr << "Error: Could not parse file " << filename << std::endl;
        return 1;
    }

    size_t size;
    void *data = xml_parsed::xml_doc_wrap(document, size);
    xmlFreeDoc(document);

    void *chunk = std::malloc(size);
    std::memcpy(chunk, data, size);
    std::free(data);

    document = (xmlDocPtr) chunk;
    document = xml_parsed::xml_doc_unwrap(chunk, size);

    xmlNode *root = xmlDocGetRootElement(document);
    std::cout << "Root Element: " << root->name << std::endl;

    printElementNames(root->children);

    std::free(chunk);
    xmlCleanupParser();

    return 0;
}
