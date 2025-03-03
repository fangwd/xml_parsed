#include <cassert>
#include <iostream>
#include <unordered_set>

#include "xml_parsed.h"

namespace xml_parsed {

#define INVALID_PTR std::ptrdiff_t(-1)

#define TO_OFFSET(base, ptr)                       \
  (ptr ? reinterpret_cast<std::ptrdiff_t>(         \
             reinterpret_cast<const char *>(ptr) - \
             reinterpret_cast<const char *>(base)) \
       : INVALID_PTR)

class Tracker {
  std::unordered_set<const void *> seen_;

 public:
  bool track(const void *ptr) {
    if (seen_.find(ptr) != seen_.end()) {
      return true;
    }
    seen_.insert(ptr);
    return false;
  }

  void clear() { seen_.clear(); }
};

class Wrapper {
  xmlDocPtr base_;
  size_t max_offset_;
  Tracker tracker_;

  std::unordered_map<const void *, const void *> addr_map_;

  template <typename T>
  void to_offset(T **ptr) {
    if (!*ptr) {
      *ptr = (T *)(-1);
    } else {
      auto it = addr_map_.find(*ptr);
      if (it != addr_map_.end()) {
        *ptr = (T *)(it->second);
      } else {
        assert((const char *)(*ptr) >= (char *)base_);
        auto value = reinterpret_cast<T *>(TO_OFFSET(base_, *ptr));
        max_offset_ = std::max(max_offset_, size_t(*ptr));
        addr_map_[*ptr] = value;
        *ptr = value;
      }
    }
  }

  void wrap(xmlNsPtr ns) {
    if (!ns || tracker_.track(ns)) return;

    to_offset(&ns->href);
    to_offset(&ns->prefix);

    wrap(ns->next);
    to_offset(&ns->next);

    to_offset(&ns->context);
  }

  void wrap(xmlAttrPtr attr) {
    if (!attr || tracker_.track(attr)) return;

    to_offset(&attr->name);

    wrap(attr->children);
    to_offset(&attr->children);

    wrap(attr->next);
    to_offset(&attr->next);

    wrap(attr->ns);
    to_offset(&attr->ns);

    to_offset(&attr->last);
    to_offset(&attr->parent);
    to_offset(&attr->prev);
    to_offset(&attr->doc);
  }

  void wrap(xmlNodePtr node) {
    if (!node || tracker_.track(node)) return;

    to_offset(&node->name);
    to_offset(&node->content);

    wrap(node->properties);
    to_offset(&node->properties);

    wrap(node->ns);
    to_offset(&node->ns);

    wrap(node->nsDef);
    to_offset(&node->nsDef);

    wrap(node->children);
    to_offset(&node->children);

    wrap(node->next);
    to_offset(&node->next);

    to_offset(&node->last);
    to_offset(&node->parent);
    to_offset(&node->prev);
    to_offset(&node->doc);
  }

  void wrap(xmlDocPtr doc) {
    if (!doc || tracker_.track(doc)) return;

    to_offset(&doc->name);
    to_offset(&doc->version);
    to_offset(&doc->encoding);
    to_offset(&doc->URL);
    to_offset(&doc->oldNs);

    wrap(doc->children);
    to_offset(&doc->children);

    wrap(doc->next);
    to_offset(&doc->next);

    to_offset(&doc->last);
    to_offset(&doc->parent);
    to_offset(&doc->prev);
    to_offset(&doc->doc);
  }

 public:
  Wrapper(xmlDocPtr doc) : base_(doc) {}

  void wrap(size_t &size) {
    tracker_.clear();
    max_offset_ = 0;
    wrap(base_);
  }
};

#define TO_PTR(base, offset)                                           \
  (reinterpret_cast<std::uintptr_t>(offset) != INVALID_PTR             \
       ? reinterpret_cast<char *>((char *)(base) +                     \
                                  reinterpret_cast<ptrdiff_t>(offset)) \
       : nullptr)

class Unwrapper {
  xmlDocPtr base_;
  size_t total_size_;
  Tracker tracker_;
  std::unordered_map<const void *, const void *> addr_map_;

  template <typename T>
  void to_ptr(T **offset) {
    if (*offset == (T *)(-1)) {
      *offset = nullptr;
    } else {
      auto it = addr_map_.find(*offset);
      if (it != addr_map_.end()) {
        *offset = (T *)(it->second);
      } else {
        auto value = reinterpret_cast<T *>(TO_PTR(base_, *offset));
        addr_map_[*offset] = value;
        *offset = value;
      }
      if ((const char *)(*offset) >= (const char *)base_ + total_size_) {
        std::cerr << "Out of bound ptr\n";
        assert(0);
      }
    }
  }

  void to_ptr(const xmlChar **offset) {
    if (*offset == (xmlChar *)(-1)) {
      *offset = nullptr;
    } else {
      auto it = addr_map_.find(*offset);
      if (it != addr_map_.end()) {
        *offset = (const xmlChar *)(it->second);
      } else {
        auto value = reinterpret_cast<const xmlChar *>(TO_PTR(base_, *offset));
        addr_map_[*offset] = value;
        *offset = value;
      }
      if ((const char *)(*offset) >= (const char *)base_ + total_size_) {
        std::cerr << "Out of bound ptr for xmlChar\n";
        assert(0);
      }
    }
  }

  void unwrap(xmlNsPtr ns) {
    if (!ns || tracker_.track(ns)) return;

    to_ptr(&ns->href);
    to_ptr(&ns->prefix);

    to_ptr(&ns->next);
    unwrap(ns->next);

    to_ptr(&ns->context);
  }

  void unwrap(xmlAttrPtr attr) {
    if (!attr || tracker_.track(attr)) return;

    to_ptr(&attr->name);

    to_ptr(&attr->children);
    unwrap(attr->children);

    to_ptr(&attr->next);
    unwrap(attr->next);

    to_ptr(&attr->ns);
    unwrap(attr->ns);

    to_ptr(&attr->last);
    to_ptr(&attr->parent);
    to_ptr(&attr->prev);
    to_ptr(&attr->doc);
  }

  void unwrap(xmlNodePtr node) {
    if (!node || tracker_.track(node)) return;

    to_ptr(&node->name);
    to_ptr(&node->content);

    to_ptr(&node->properties);
    unwrap(node->properties);

    to_ptr(&node->ns);
    unwrap(node->ns);

    to_ptr(&node->nsDef);
    unwrap(node->nsDef);

    to_ptr(&node->children);
    unwrap(node->children);

    to_ptr(&node->next);
    unwrap(node->next);

    to_ptr(&node->last);
    to_ptr(&node->parent);
    to_ptr(&node->prev);
    to_ptr(&node->doc);
  }

  void unwrap(xmlDocPtr doc) {
    if (!doc || tracker_.track(doc)) return;

    to_ptr(&doc->name);
    to_ptr(&doc->version);
    to_ptr(&doc->encoding);
    to_ptr(&doc->URL);
    to_ptr(&doc->oldNs);

    to_ptr(&doc->children);
    unwrap(doc->children);

    to_ptr(&doc->next);
    unwrap(doc->next);

    to_ptr(&doc->last);
    to_ptr(&doc->parent);
    to_ptr(&doc->prev);
    to_ptr(&doc->doc);
  }

 public:
  Unwrapper(void *data, size_t size)
      : base_((xmlDocPtr)data), total_size_(size) {}

  void unwrap() {
    tracker_.clear();
    unwrap(base_);
  }
};

void *xml_doc_wrap(const xmlDocPtr doc, size_t &size) {
  xmlDocPtr copy = xml_doc_copy(doc, size);
  Wrapper wrapper(copy);
  size_t result_size;
  wrapper.wrap(result_size);
  if (result_size >= size) {
    assert(0);
  }
  return (void *)copy;
}

xmlDocPtr xml_doc_unwrap(void *data, size_t size) {
  Unwrapper unwrapper(data, size);
  unwrapper.unwrap();
  return (xmlDocPtr)data;
}

}  // namespace xml_parsed
