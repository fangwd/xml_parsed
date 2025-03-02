#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <unordered_set>

#include "xml_parsed.h"

namespace xml_parsed {

class Estimator {
    const xmlDocPtr doc_;
    std::unordered_set<const void *> seen_;

    bool has(const void *ptr) {
        return seen_.find(ptr) != seen_.end();
    }

    void add(const void *ptr) {
        seen_.insert(ptr);
    }

    size_t estimate_string(const xmlChar *str) {
        return str ? ALIGN(xmlStrlen(str) + 1) : 0;
    }

    size_t calculate(const xmlNsPtr ns) {
        size_t total = 0;

        total += estimate_string(ns->href);
        total += estimate_string(ns->prefix);
        total += estimate(ns->next);

        return total;
    }

    size_t calculate(const xmlAttrPtr attr) {
        size_t total = 0;

        total += estimate_string(attr->name);
        total += estimate(attr->children);
        total += estimate(attr->next);
        total += estimate(attr->ns);

        return total;
    }

    size_t calculate(const xmlNodePtr node) {
        size_t total = 0;

        total += estimate_string(node->name);
        total += estimate_string(node->content);
        total += estimate(node->properties);
        total += estimate(node->ns);
        total += estimate(node->nsDef);

        total += estimate(node->children);
        total += estimate(node->next);

        return total;
    }

    size_t calculate(const xmlDocPtr doc) {
        size_t total = 0;

        total += estimate_string((const xmlChar *)doc->name);
        total += estimate_string(doc->version);
        total += estimate_string(doc->encoding);
        total += estimate_string(doc->URL);
        total += estimate(doc->oldNs);

        total += estimate(doc->children);
        total += estimate(doc->next);

        return total;
    }

    template <typename T>
    size_t estimate(const T obj) {
        if (!obj || has(obj)) {
            return 0;
        }

        add(obj);

        size_t base_size = ALIGN(sizeof(*obj));
        size_t extra_size = calculate(obj);

        return base_size + extra_size;
    }

   public:
    Estimator(const xmlDocPtr doc) : doc_(doc) {}

    size_t estimate() {
        return estimate(doc_);
    }
};

size_t size_of_xml_doc(const xmlDocPtr doc) {
    Estimator estimator(doc);
    return estimator.estimate();
}

}  // namespace xml_parsed