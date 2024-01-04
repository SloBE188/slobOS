#ifndef PATHPARSER_H
#define PATHPARSER_H


//This struct represents the root oft the path that is being parsed.
struct path_root
{
    int drive_no;
    struct path_part* first;
};


//This struct represents a part of the path. Eacht part containts a astring aprt which contains the name of the direcory or file at that part of the path and a pointer to the next part of the path.
struct path_part
{
    const char* part;
    struct path_part* next;
};
/*
@param path: path to be parsed
@param current directory path: the path of the current directory
It returns a pointer to a pathr_root struct representing the parsed path.*/
struct path_root* pathparser_parse(const char* path, const char* current_directory_path);
void pathparser_free(struct path_root* root);

#endif