#pragma once

#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <cstdint>
#include <cstddef>

namespace xml_parsed {

#define ALIGN(p) (((uintptr_t)(p) + (alignof(std::max_align_t) - 1)) & ~(uintptr_t)(alignof(std::max_align_t) - 1))

size_t size_of_xml_doc(const xmlDocPtr doc);
xmlDocPtr xml_doc_copy(const xmlDocPtr doc, size_t&);

void *xml_doc_wrap(const xmlDocPtr, size_t &);
xmlDocPtr xml_doc_unwrap(void *, size_t);

}  // namespace xml_parsed
