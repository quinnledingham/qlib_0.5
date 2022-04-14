internal Image
LoadImage(const char* FileName)
{
    Image NewImage;
    NewImage.data = stbi_load(FileName, &NewImage.x, &NewImage.y, &NewImage.n, 4);
    
    Assert(NewImage.data != 0); // Image loaded
    
    return NewImage;
}

internal Image
ResizeImage(Image ToResize, int Width, int Height)
{
    Image ResizedImage;
    stbir_resize_uint8(ToResize.data, ToResize.x, ToResize.y, 0, ResizedImage.data, Width, Height, 0, ToResize.n);
    
    ResizedImage.x = Width;
    ResizedImage.y = Height;
    
    stbi_image_free(ToResize.data);
    
    return ResizedImage;
}

internal void
SaveImage(const char* SaveFileName)
{
    
}