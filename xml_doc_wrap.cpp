#include "xml_parsed.h"

namespace xml_parsed {
class Wrapper {
    xmlDocPtr doc_;

   public:
    Wrapper(xmlDocPtr doc) : doc_(doc) {
    }

    void wrap();
};

void *xml_doc_wrap(const xmlDocPtr doc, size_t &size) {
    xmlDocPtr copy = xml_doc_copy(doc, size);
    Wrapper wrapper(copy);
    wrapper.wrap();
    return (void *)copy;
}

xmlDocPtr xml_doc_unwrap(void *, size_t);
}  // namespace xml_parsed
