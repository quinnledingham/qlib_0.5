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
    int x;
    int y;
    int n;
};

#endif //IMAGE_H