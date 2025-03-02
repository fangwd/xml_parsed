#include <assert.h>

#include <unordered_map>

#include "xml_parsed.h"

namespace xml_parsed {

#define XMLCOPY_DEBUG 1

class Duplicator {
    const xmlDocPtr doc_;
    char *memory_;
    size_t allocated_;
    size_t total_size_;

    std::unordered_map<const void *, void *> copy_map_;

    void *allocate(size_t size) {
        size = ALIGN(size);
        if (allocated_ + size > total_size_) {
            throw std::bad_alloc();
        }
        char *result = memory_ + allocated_;
        allocated_ += size;
        return (void *)result;
    }

    void *get(const void *src) {
        auto it = copy_map_.find(src);
        return it == copy_map_.end() ? nullptr : it->second;
    }

    void set(const void *src, void *dst) {
        copy_map_[src] = dst;
    }

    xmlChar *copy_string(const xmlChar *src) {
        if (src) {
            size_t size = xmlStrlen(src) + 1;
            auto dst = (xmlChar *)allocate(size);
            memcpy(dst, src, size);
            return dst;
        }
        return nullptr;
    }

    xmlNsPtr copy(const xmlNsPtr src, xmlNsPtr dst) {
        memcpy(dst, src, sizeof(*src));

        dst->href = copy_string(src->href);
        dst->prefix = copy_string(src->prefix);
        dst->next = copy(src->next);

        return dst;
    }

    xmlAttrPtr copy(const xmlAttrPtr src, xmlAttrPtr dst) {
        memcpy(dst, src, sizeof(*src));

        dst->_private = nullptr;
        dst->name = copy_string(src->name);
        dst->children = copy(src->children);
        dst->next = copy(src->next);
        dst->ns = copy(src->ns);

        return dst;
    }

    xmlNodePtr copy(const xmlNodePtr src, xmlNodePtr dst) {
        memcpy(dst, src, sizeof(*src));

        dst->name = copy_string(src->name);
        dst->content = copy_string(src->content);
        dst->properties = copy(src->properties);
        dst->ns = copy(src->ns);
        dst->nsDef = copy(src->nsDef);

        dst->children = copy(src->children);
        dst->next = copy(src->next);

        return dst;
    }

    xmlDocPtr copy(const xmlDocPtr src, xmlDocPtr dst) {
        memcpy(dst, src, sizeof(*src));

        dst->name = (char *)copy_string((xmlChar *)src->name);
        dst->version = copy_string(src->version);
        dst->encoding = copy_string(src->encoding);
        dst->URL = copy_string(src->URL);
        dst->oldNs = copy(src->oldNs);

        dst->children = copy(src->children);
        dst->next = copy(src->next);

        dst->intSubset = nullptr;
        dst->extSubset = nullptr;
        dst->ids = nullptr;
        dst->refs = nullptr;
        dst->dict = nullptr;
        dst->psvi = nullptr;

        return dst;
    }

    template <typename T>
    T copy(const T src) {
        auto dst = (T)get(src);
        if (dst || !src) {
            return dst;
        }

        dst = (T)allocate(sizeof(*src));

        set(src, dst);

        copy(src, dst);

        return dst;
    }

   public:
    Duplicator(const xmlDocPtr doc) : doc_(doc), memory_(nullptr), allocated_(0), total_size_(0) {}

    xmlDocPtr duplicate() {
        copy_map_.clear();
        total_size_ = size_of_xml_doc(doc_);
        memory_ = (char *)std::malloc(total_size_);
        if (!memory_) return nullptr;
        allocated_ = 0;
        auto dst = copy(doc_);
        patch(dst, doc_);
        return dst;
    }

    size_t total_size() {
        return total_size_;
    }

    size_t space_size() {
        return total_size_ - allocated_;
    }

   private:
    void patch(xmlNsPtr dst, const xmlNsPtr src) {
        if (!dst) {
            return;
        }
        dst->context = (xmlDocPtr)get(src->context);
        assert(!src->context || dst->context);
    }

    void patch(xmlAttrPtr dst, const xmlAttrPtr src) {
        if (!dst) {
            return;
        }

        dst->last = (xmlNodePtr)get(src->last);
        dst->parent = (xmlNodePtr)get(src->parent);
        dst->prev = (xmlAttrPtr)get(src->prev);
        dst->doc = (xmlDocPtr)get(src->doc);

#if (XMLCOPY_DEBUG)
        assert(!src->last || dst->last);
        assert(!src->parent || dst->parent);
        assert(!src->prev || dst->prev);
        assert(!src->doc || dst->doc);
#endif
        patch(dst->children, src->children);
        patch(dst->next, src->next);

        patch(dst->ns, src->ns);
    }

    void patch(xmlNodePtr dst, const xmlNodePtr src) {
        if (!dst) {
            return;
        }

        dst->last = (xmlNodePtr)get(src->last);
        dst->parent = (xmlNodePtr)get(src->parent);
        dst->prev = (xmlNodePtr)get(src->prev);
        dst->doc = (xmlDocPtr)get(src->doc);

#if (XMLCOPY_DEBUG)
        assert(!src->last || dst->last);
        assert(!src->parent || dst->parent);
        assert(!src->prev || dst->prev);
        assert(!src->doc || dst->doc);
#endif

        patch(dst->children, src->children);
        patch(dst->next, src->next);

        patch(dst->ns, src->ns);
        patch(dst->properties, src->properties);
        patch(dst->nsDef, src->nsDef);
    }

    void patch(xmlDocPtr dst, const xmlDocPtr src) {
        if (!dst) {
            return;
        }

        dst->last = (xmlNodePtr)get(src->last);
        dst->parent = (xmlNodePtr)get(src->parent);
        dst->prev = (xmlNodePtr)get(src->prev);
        dst->doc = (xmlDocPtr)get(src->doc);

#if (XMLCOPY_DEBUG)
        assert(!src->last || dst->last);
        assert(!src->parent || dst->parent);
        assert(!src->prev || dst->prev);
        assert(!src->doc || dst->doc);
#endif

        patch(dst->children, src->children);
        patch(dst->next, src->next);
    }
};

xmlDocPtr xml_doc_copy(const xmlDocPtr doc, size_t& size) {
    Duplicator duplicator(doc);
    auto copy = duplicator.duplicate();
    size = duplicator.total_size();
    assert(duplicator.space_size() == 0);
    return copy;
}

}