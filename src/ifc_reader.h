#pragma once

#include <ifcparse/IfcParse.h>

class OttIFCloader
{
public:
    OttIFCloader(const std::string& filename);
    ~OttIFCloader();

    void readFile();
    void printEntities(); 

private:
    std::string filename;
    IfcParse::IfcFile* ifcFile;
};