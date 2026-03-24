#pragma once
#include <windows.h>
#include <d3d9.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>

namespace ImageLoader {

    // Loads a PNG/JPG texture from disk into a DirectX 9 Texture.
    // Includes extensive error handling as per professional directives.
    inline bool LoadTextureFromFile(const char* filename, IDirect3DDevice9* pDevice, IDirect3DTexture9** out_texture, int* out_width, int* out_height) {
        if (!pDevice) return false;
        
        // Load the image with stb_image
        int image_width = 0;
        int image_height = 0;
        int image_channels = 0;
        unsigned char* image_data = stbi_load(filename, &image_width, &image_height, &image_channels, 4);
        
        if (image_data == NULL) {
            // Log missing texture (silent fail to avoid crashing the overlay, just returns false)
            return false;
        }

        // Create texture
        IDirect3DTexture9* texture = NULL;
        HRESULT hr = pDevice->CreateTexture(image_width, image_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL);
        if (FAILED(hr)) {
            stbi_image_free(image_data);
            return false;
        }

        // Lock the texture to copy raw pixels
        D3DLOCKED_RECT locked_rect;
        if (texture->LockRect(0, &locked_rect, NULL, 0) == D3D_OK) {
            for (int y = 0; y < image_height; y++) {
                const unsigned char* src = image_data + (y * image_width * 4);
                unsigned char* dst = (unsigned char*)locked_rect.pBits + (y * locked_rect.Pitch);
                for (int x = 0; x < image_width; x++) {
                    dst[x * 4 + 0] = src[x * 4 + 2]; // Blue
                    dst[x * 4 + 1] = src[x * 4 + 1]; // Green
                    dst[x * 4 + 2] = src[x * 4 + 0]; // Red
                    dst[x * 4 + 3] = src[x * 4 + 3]; // Alpha
                }
            }
            texture->UnlockRect(0);
        } else {
            texture->Release();
            stbi_image_free(image_data);
            return false;
        }

        stbi_image_free(image_data);

        *out_texture = texture;
        if (out_width) *out_width = image_width;
        if (out_height) *out_height = image_height;

        return true;
    }
}
