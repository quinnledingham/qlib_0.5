#ifndef IMAGE_H
#define IMAGE_H

struct Image
{
    int x;
    int y;
    int n;
    unsigned char* data;
};

struct ImageHeader
{
    unsigned int x;
    unsigned int y;
    unsigned int n;
};

#endif //IMAGE_H