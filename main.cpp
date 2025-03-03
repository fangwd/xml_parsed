#include <fcntl.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iostream>

#include "xml_parsed.h"

static int serialise(const char *html_file, const char *output_file) {
  xmlDocPtr doc =
      htmlReadFile(html_file, "utf-8", HTML_PARSE_RECOVER | HTML_PARSE_NOERROR);

  size_t size;
  void *data = xml_parsed::xml_doc_wrap(doc, size);
  xmlFreeDoc(doc);

  std::ofstream file(output_file, std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file for writing: " << output_file << std::endl;
    return 1;
  }

  file.write((char *)data, size);
  file.close();

  std::free(data);

  return 0;
}

static int evaluate_xpath(const char *html_file, const xmlChar *xpath_expr) {
  auto start = std::chrono::high_resolution_clock::now();
  xmlDocPtr doc =
      htmlReadFile(html_file, "utf-8", HTML_PARSE_RECOVER | HTML_PARSE_NOERROR);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  auto ctxt = xmlXPathNewContext(doc);
  auto xpath = xmlXPathEvalExpression(xpath_expr, ctxt);

  auto nodes = xpath->nodesetval;
  auto size = (nodes) ? nodes->nodeNr : 0;

  std::cout << "Time to parse: " << elapsed.count()
            << " seconds; result count: " << size << "\n";

  xmlXPathFreeObject(xpath);
  xmlXPathFreeContext(ctxt);

  xmlFreeDoc(doc);

  return 0;
}

char *load_file(const char *filename, size_t &size) {
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    perror("open");
    return nullptr;
  }

  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    perror("fstat");
    close(fd);
    return nullptr;
  }
  size = sb.st_size;

  char *data =
      static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
  close(fd);

  if (data == MAP_FAILED) {
    perror("mmap");
    return nullptr;
  }
  return data;
}

int evaluate_xpath_parsed(const char *parsed_file, const xmlChar *xpath_expr) {
  auto start = std::chrono::high_resolution_clock::now();

  std::ifstream file(parsed_file, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("Failed to open file");
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size)) {
    throw std::runtime_error("Failed to read file");
  }

  xmlDocPtr doc = xml_parsed::xml_doc_unwrap(buffer.data());

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  auto ctxt = xmlXPathNewContext(doc);
  auto xpath = xmlXPathEvalExpression(xpath_expr, ctxt);

  auto nodes = xpath->nodesetval;
  auto count = (nodes) ? nodes->nodeNr : 0;

  std::cout << "Time to parse: " << elapsed.count()
            << " seconds; result count: " << count << "\n";

  xmlXPathFreeObject(xpath);
  xmlXPathFreeContext(ctxt);

  return 0;
}

int main(int argc, char **argv) {
  bool do_serialise = false;
  bool parsed_input = false;

  const char *input_file = nullptr;
  const char *output_file = nullptr;
  const char *xpath_expr = nullptr;

  for (int i = 1; i < argc; i++) {
    std::string option = std::string(argv[i]);
    if (option == "--serialise" || option == "-s") {
      input_file = argv[++i];
      do_serialise = true;
    } else if (option == "--output" || option == "-o") {
      output_file = argv[++i];
    } else if (option == "--evaluate" || option == "-e") {
      xpath_expr = argv[++i];
    } else if (option == "--html") {
      input_file = argv[++i];
    } else if (option == "--parsed") {
      input_file = argv[++i];
      parsed_input = true;
    }
  }

  xmlInitParser();

  if (do_serialise) {
    if (input_file && output_file) {
      serialise(input_file, output_file);
    } else {
      std::cerr << "Missing input/output file name(s)\n";
      return 1;
    }
  } else if (xpath_expr) {
    if (parsed_input) {
      evaluate_xpath_parsed(input_file, (const xmlChar *)xpath_expr);
    } else {
      evaluate_xpath(input_file, (const xmlChar *)xpath_expr);
    }
  }

  xmlCleanupParser();

  return 0;
}
