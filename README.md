## About

This is work in progress. 
- ORC support is yet to be done.
- Asynchronous download from Crossref is to be implemented.

MetaSci is a project aimed at [downloading and] and parsing Crossref's JSONs, containing 
scientific publications metadata. Crossref is one of the leading registration authorities,
having approximately 80% of the market share. 
The files are then converted to Apache's ORC format, which is storage-efficient and 
suitable for download to Hadoop or BigQuery. I'm also going to use them for BI later.

## Sources

The main file is `crossref_parser.cpp`. Most of the required declarations and definitions
are contained in `article.h` (yes, I use it as a mix of a header and a source file, and I
know it's a non-standard approach; however, I think it's justified by the project's 
size). 

## Building

Use `Cmake` to build the project: in the project's root directory execute
```bash
cmake . 
cmake --build .
```

