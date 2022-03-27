cgltf_data* LoadGLTFFile(const char* path)
{
    cgltf_options options;
    memset(&options, 0, sizeof(cgltf_options));
    cgltf_data* data = NULL;
    cgltf_result = result = cgltf_parse_file(&options, path, &data);
    
    if (result != cgltf_result_success)
    {
        printo(S() + "Could not load: " + path + "\n");
        return 0;
    }
    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) 
    {
        cgltf_free(data);
        printo(S() + "Could not load: " + path + "\n");
        return 0;
    }
    result = cgltf_validate(data);
    if (result != cgltf_result_success)
    {
        cgltf_free(data);
        printo("Invalid file: " + path + "\n");
        return 0;
    }
    return data;
}

void FreeGLTFFile(cgltf_data* data)
{
    if (data == 0)
    {
        printo("WARNING: Can't free null data\n");
    }
    else
    {
        cgltf_free(data);
    }
}