`xml_parsed` is a small C++ library that helps serialise XML documents parsed with
[libxml2](https://gitlab.gnome.org/GNOME/libxml2/) into memory chunks and deserialise
memory chunks back into XML documents.

## Usage

To serialise an `xmlDocPtr` (`doc`):

```cpp
#include "xml_parsed.h"

size_t size;
void *data = xml_parsed::wrap(doc, size)

// deal with data of size bytes (e.g. write it to a file)
// ...

std::free(data);
```

To deserialise an `xmlDocPtr` from a chunk of memory:

```cpp
#include "xml_parsed.h"

size_t size;
void *data;

// Initialise data and size by e.g. reading from a file
// size = ...
// data = ...

xmlDocPtr doc = xml_parsed::unwrap(data)

// deal with doc as usual (e.g. evaluate an xpath etc)

std::free(data);
```


## How to build
```
cmake -DCMAKE_BUILD_TYPE=Debug -B build .
cmake --build build
```

Note: on MacOS, you may need to install `pkg-config` by running:
```
brew install pkg-config
```

## CLI

To serialise a HTML file:
```
./build/parsed --serialise files/example.html --output files/example.dat
```

To evaluate an xpath against a html doc:
```
./build/parsed --html files/example.html  -e '//*[last()]' 
```

To evaluate an xpath against a parsed xml doc:
```
./build/parsed --parsed files/example.dat -e '//*[last()]' 
```
