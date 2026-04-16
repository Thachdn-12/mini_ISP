#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// =============================================================================
// Basic Image Data Structures
// =============================================================================

typedef struct {
    unsigned char r, g, b, a;  // RGBA format
} Pixel;

typedef struct {
    Pixel* pixels;
    int width;
    int height;
    int stride;  // bytes per row
    size_t pixel_count;
} ImageBuffer;

// Create a new image buffer
ImageBuffer* create_image_buffer(int width, int height) {
    ImageBuffer* img = (ImageBuffer*)malloc(sizeof(ImageBuffer));
    if (!img) return NULL;
    
    img->pixel_count = width * height;
    img->pixels = (Pixel*)malloc(img->pixel_count * sizeof(Pixel));
    if (!img->pixels) {
        free(img);
        return NULL;
    }
    
    img->width = width;
    img->height = height;
    img->stride = width * 4;
    
    // Initialize pixels to black
    for (size_t i = 0; i < img->pixel_count; ++i) {
        img->pixels[i].r = 0;
        img->pixels[i].g = 0;
        img->pixels[i].b = 0;
        img->pixels[i].a = 255;
    }
    
    return img;
}

// Free image buffer
void free_image_buffer(ImageBuffer* img) {
    if (img) {
        if (img->pixels) {
            free(img->pixels);
        }
        free(img);
    }
}

// Get pixel at position
Pixel* get_pixel(ImageBuffer* img, int x, int y) {
    if (!img || x < 0 || x >= img->width || y < 0 || y >= img->height) {
        return NULL;
    }
    return &img->pixels[y * img->width + x];
}

// =============================================================================
// ISP Pipeline Base Structures
// =============================================================================

// Forward declaration
typedef struct ISPModule ISPModule;

// Function pointer types for module operations
typedef void (*process_func)(ISPModule* module, ImageBuffer* input, ImageBuffer* output);
typedef void (*configure_func)(ISPModule* module, const char* param, float value);
typedef void (*destroy_func)(ISPModule* module);

// Base structure for ISP processing modules
struct ISPModule {
    char name[64];
    int enabled;
    
    // Function pointers
    process_func process;
    configure_func configure;
    destroy_func destroy;
    
    // Module-specific data (to be allocated by each module)
    void* data;
};

// ISP Pipeline Manager
typedef struct {
    ISPModule** modules;
    int module_count;
    int module_capacity;
    char name[64];
} ISPPipeline;

// Create a new ISP pipeline
ISPPipeline* create_isp_pipeline(const char* name) {
    ISPPipeline* pipeline = (ISPPipeline*)malloc(sizeof(ISPPipeline));
    if (!pipeline) return NULL;
    
    pipeline->modules = (ISPModule**)malloc(16 * sizeof(ISPModule*));  // Initial capacity
    if (!pipeline->modules) {
        free(pipeline);
        return NULL;
    }
    
    pipeline->module_count = 0;
    pipeline->module_capacity = 16;
    strncpy(pipeline->name, name, sizeof(pipeline->name) - 1);
    pipeline->name[sizeof(pipeline->name) - 1] = '\0';
    
    return pipeline;
}

// Add module to pipeline
int add_module_to_pipeline(ISPPipeline* pipeline, ISPModule* module) {
    if (!pipeline || !module) return 0;
    
    // Resize if needed
    if (pipeline->module_count >= pipeline->module_capacity) {
        int new_capacity = pipeline->module_capacity * 2;
        ISPModule** new_modules = (ISPModule**)realloc(pipeline->modules, 
                                                      new_capacity * sizeof(ISPModule*));
        if (!new_modules) return 0;
        
        pipeline->modules = new_modules;
        pipeline->module_capacity = new_capacity;
    }
    
    pipeline->modules[pipeline->module_count] = module;
    pipeline->module_count++;
    
    return 1;
}

// Free ISP pipeline
void free_isp_pipeline(ISPPipeline* pipeline) {
    if (pipeline) {
        // Destroy all modules
        for (int i = 0; i < pipeline->module_count; ++i) {
            if (pipeline->modules[i] && pipeline->modules[i]->destroy) {
                pipeline->modules[i]->destroy(pipeline->modules[i]);
            }
        }
        
        if (pipeline->modules) {
            free(pipeline->modules);
        }
        free(pipeline);
    }
}

// =============================================================================
// ISP Processing Modules Implementation
// =============================================================================

// Module-specific data structures
typedef struct {
    int black_level;
} BlackLevelData;

typedef struct {
    float center_gain;
    float corner_gain;
} LensShadingData;

typedef struct {
    float red_gain;
    float green_gain;
    float blue_gain;
} WhiteBalanceData;

typedef struct {
    float gamma;
    unsigned char gamma_table[256];
} GammaData;

typedef struct {
    float strength;
} NoiseReductionData;

// 1. Black Level Correction Module
void black_level_process(ISPModule* module, ImageBuffer* input, ImageBuffer* output) {
    if (!module || !input || !output || !module->data) return;
    
    BlackLevelData* data = (BlackLevelData*)module->data;
    
    // Ensure output size matches input
    if (input->width != output->width || input->height != output->height) {
        printf("Warning: Output buffer size mismatch\n");
        return;
    }
    
    for (size_t i = 0; i < input->pixel_count; ++i) {
        // Subtract black level from each channel
        int r = (int)input->pixels[i].r - data->black_level;
        int g = (int)input->pixels[i].g - data->black_level;
        int b = (int)input->pixels[i].b - data->black_level;
        
        output->pixels[i].r = (unsigned char)(r > 0 ? r : 0);
        output->pixels[i].g = (unsigned char)(g > 0 ? g : 0);
        output->pixels[i].b = (unsigned char)(b > 0 ? b : 0);
        output->pixels[i].a = input->pixels[i].a;
    }
}

void black_level_configure(ISPModule* module, const char* param, float value) {
    if (!module || !module->data || !param) return;
    
    BlackLevelData* data = (BlackLevelData*)module->data;
    
    if (strcmp(param, "black_level") == 0) {
        data->black_level = (int)value;
    }
}

void black_level_destroy(ISPModule* module) {
    if (module) {
        if (module->data) {
            free(module->data);
        }
        free(module);
    }
}

ISPModule* create_black_level_module() {
    ISPModule* module = (ISPModule*)malloc(sizeof(ISPModule));
    if (!module) return NULL;
    
    BlackLevelData* data = (BlackLevelData*)malloc(sizeof(BlackLevelData));
    if (!data) {
        free(module);
        return NULL;
    }
    
    // Initialize data
    data->black_level = 64;
    
    // Initialize module
    strncpy(module->name, "BlackLevelCorrection", sizeof(module->name) - 1);
    module->name[sizeof(module->name) - 1] = '\0';
    module->enabled = 1;
    module->process = black_level_process;
    module->configure = black_level_configure;
    module->destroy = black_level_destroy;
    module->data = data;
    
    return module;
}

// 2. White Balance Module
void white_balance_process(ISPModule* module, ImageBuffer* input, ImageBuffer* output) {
    if (!module || !input || !output || !module->data) return;
    
    WhiteBalanceData* data = (WhiteBalanceData*)module->data;
    
    // Ensure output size matches input
    if (input->width != output->width || input->height != output->height) {
        printf("Warning: Output buffer size mismatch\n");
        return;
    }
    
    for (size_t i = 0; i < input->pixel_count; ++i) {
        const Pixel* src = &input->pixels[i];
        Pixel* dst = &output->pixels[i];
        
        // Apply color gains
        float r = src->r * data->red_gain;
        float g = src->g * data->green_gain;
        float b = src->b * data->blue_gain;
        
        dst->r = (unsigned char)(r > 255.0f ? 255.0f : r);
        dst->g = (unsigned char)(g > 255.0f ? 255.0f : g);
        dst->b = (unsigned char)(b > 255.0f ? 255.0f : b);
        dst->a = src->a;
    }
}

void white_balance_configure(ISPModule* module, const char* param, float value) {
    if (!module || !module->data || !param) return;
    
    WhiteBalanceData* data = (WhiteBalanceData*)module->data;
    
    if (strcmp(param, "red_gain") == 0) {
        data->red_gain = value;
    } else if (strcmp(param, "green_gain") == 0) {
        data->green_gain = value;
    } else if (strcmp(param, "blue_gain") == 0) {
        data->blue_gain = value;
    }
}

void white_balance_destroy(ISPModule* module) {
    if (module) {
        if (module->data) {
            free(module->data);
        }
        free(module);
    }
}

ISPModule* create_white_balance_module() {
    ISPModule* module = (ISPModule*)malloc(sizeof(ISPModule));
    if (!module) return NULL;
    
    WhiteBalanceData* data = (WhiteBalanceData*)malloc(sizeof(WhiteBalanceData));
    if (!data) {
        free(module);
        return NULL;
    }
    
    // Initialize data
    data->red_gain = 1.2f;
    data->green_gain = 1.0f;
    data->blue_gain = 1.1f;
    
    // Initialize module
    strncpy(module->name, "WhiteBalance", sizeof(module->name) - 1);
    module->name[sizeof(module->name) - 1] = '\0';
    module->enabled = 1;
    module->process = white_balance_process;
    module->configure = white_balance_configure;
    module->destroy = white_balance_destroy;
    module->data = data;
    
    return module;
}

// 3. Gamma Correction Module
void update_gamma_table(GammaData* data) {
    for (int i = 0; i < 256; ++i) {
        float normalized = i / 255.0f;
        float corrected = powf(normalized, 1.0f / data->gamma);
        data->gamma_table[i] = (unsigned char)(corrected * 255.0f);
    }
}

void gamma_process(ISPModule* module, ImageBuffer* input, ImageBuffer* output) {
    if (!module || !input || !output || !module->data) return;
    
    GammaData* data = (GammaData*)module->data;
    
    // Ensure output size matches input
    if (input->width != output->width || input->height != output->height) {
        printf("Warning: Output buffer size mismatch\n");
        return;
    }
    
    for (size_t i = 0; i < input->pixel_count; ++i) {
        const Pixel* src = &input->pixels[i];
        Pixel* dst = &output->pixels[i];
        
        dst->r = data->gamma_table[src->r];
        dst->g = data->gamma_table[src->g];
        dst->b = data->gamma_table[src->b];
        dst->a = src->a;
    }
}

void gamma_configure(ISPModule* module, const char* param, float value) {
    if (!module || !module->data || !param) return;
    
    GammaData* data = (GammaData*)module->data;
    
    if (strcmp(param, "gamma") == 0) {
        data->gamma = value;
        update_gamma_table(data);
    }
}

void gamma_destroy(ISPModule* module) {
    if (module) {
        if (module->data) {
            free(module->data);
        }
        free(module);
    }
}

ISPModule* create_gamma_module() {
    ISPModule* module = (ISPModule*)malloc(sizeof(ISPModule));
    if (!module) return NULL;
    
    GammaData* data = (GammaData*)malloc(sizeof(GammaData));
    if (!data) {
        free(module);
        return NULL;
    }
    
    // Initialize data
    data->gamma = 2.2f;
    update_gamma_table(data);
    
    // Initialize module
    strncpy(module->name, "GammaCorrection", sizeof(module->name) - 1);
    module->name[sizeof(module->name) - 1] = '\0';
    module->enabled = 1;
    module->process = gamma_process;
    module->configure = gamma_configure;
    module->destroy = gamma_destroy;
    module->data = data;
    
    return module;
}

// 4. Noise Reduction Module
void noise_reduction_process(ISPModule* module, ImageBuffer* input, ImageBuffer* output) {
    if (!module || !input || !output || !module->data) return;
    
    NoiseReductionData* data = (NoiseReductionData*)module->data;
    
    // Ensure output size matches input
    if (input->width != output->width || input->height != output->height) {
        printf("Warning: Output buffer size mismatch\n");
        return;
    }
    
    float strength = data->strength > 1.0f ? 1.0f : (data->strength < 0.0f ? 0.0f : data->strength);
    
    // Simple spatial noise reduction (3x3 averaging with weight)
    for (int y = 0; y < input->height; ++y) {
        for (int x = 0; x < input->width; ++x) {
            float r_sum = 0, g_sum = 0, b_sum = 0;
            float weight_sum = 0;
            
            // 3x3 neighborhood
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    
                    // Check bounds
                    if (nx >= 0 && nx < input->width && ny >= 0 && ny < input->height) {
                        float weight = (dx == 0 && dy == 0) ? 1.0f : (1.0f - strength);
                        const Pixel* pixel = get_pixel(input, nx, ny);
                        
                        r_sum += pixel->r * weight;
                        g_sum += pixel->g * weight;
                        b_sum += pixel->b * weight;
                        weight_sum += weight;
                    }
                }
            }
            
            Pixel* dst = get_pixel(output, x, y);
            const Pixel* original = get_pixel(input, x, y);
            
            dst->r = (unsigned char)(r_sum / weight_sum);
            dst->g = (unsigned char)(g_sum / weight_sum);
            dst->b = (unsigned char)(b_sum / weight_sum);
            dst->a = original->a;
        }
    }
}

void noise_reduction_configure(ISPModule* module, const char* param, float value) {
    if (!module || !module->data || !param) return;
    
    NoiseReductionData* data = (NoiseReductionData*)module->data;
    
    if (strcmp(param, "strength") == 0) {
        data->strength = value > 1.0f ? 1.0f : (value < 0.0f ? 0.0f : value);
    }
}

void noise_reduction_destroy(ISPModule* module) {
    if (module) {
        if (module->data) {
            free(module->data);
        }
        free(module);
    }
}

ISPModule* create_noise_reduction_module() {
    ISPModule* module = (ISPModule*)malloc(sizeof(ISPModule));
    if (!module) return NULL;
    
    NoiseReductionData* data = (NoiseReductionData*)malloc(sizeof(NoiseReductionData));
    if (!data) {
        free(module);
        return NULL;
    }
    
    // Initialize data
    data->strength = 0.3f;
    
    // Initialize module
    strncpy(module->name, "NoiseReduction", sizeof(module->name) - 1);
    module->name[sizeof(module->name) - 1] = '\0';
    module->enabled = 1;
    module->process = noise_reduction_process;
    module->configure = noise_reduction_configure;
    module->destroy = noise_reduction_destroy;
    module->data = data;
    
    return module;
}

// =============================================================================
// Pipeline Processing Functions
// =============================================================================

// Process image through entire pipeline
int process_image_through_pipeline(ISPPipeline* pipeline, ImageBuffer* input, ImageBuffer* output) {
    if (!pipeline || !input || !output) return 0;
    
    printf("[ISPPipeline] Processing image through %d modules...\n", pipeline->module_count);
    
    // Use input as starting point
    ImageBuffer* currentBuffer = input;
    ImageBuffer* tempBuffer = create_image_buffer(input->width, input->height);
    if (!tempBuffer) return 0;
    
    // Process through each module
    for (int i = 0; i < pipeline->module_count; ++i) {
        ISPModule* module = pipeline->modules[i];
        if (module && module->enabled && module->process) {
            printf("  [Module %d] %s\n", i, module->name);
            
            // Alternate between input/output buffers to avoid copying
            if (i == pipeline->module_count - 1) {
                // Last module writes to final output
                module->process(module, currentBuffer, output);
            } else {
                // Intermediate modules use temp buffer
                module->process(module, currentBuffer, tempBuffer);
                currentBuffer = tempBuffer;
            }
        }
    }
    
    free_image_buffer(tempBuffer);
    printf("[ISPPipeline] Processing completed!\n");
    return 1;
}

// Get module by name
ISPModule* get_module_by_name(ISPPipeline* pipeline, const char* name) {
    if (!pipeline || !name) return NULL;
    
    for (int i = 0; i < pipeline->module_count; ++i) {
        if (pipeline->modules[i] && strcmp(pipeline->modules[i]->name, name) == 0) {
            return pipeline->modules[i];
        }
    }
    return NULL;
}

// Enable module
void enable_module(ISPPipeline* pipeline, const char* name) {
    ISPModule* module = get_module_by_name(pipeline, name);
    if (module) {
        module->enabled = 1;
        printf("[ISPPipeline] Enabled module: %s\n", name);
    }
}

// Disable module
void disable_module(ISPPipeline* pipeline, const char* name) {
    ISPModule* module = get_module_by_name(pipeline, name);
    if (module) {
        module->enabled = 0;
        printf("[ISPPipeline] Disabled module: %s\n", name);
    }
}

// =============================================================================
// Utility Functions
// =============================================================================

// Create a test image with gradient
ImageBuffer* create_test_image(int width, int height) {
    ImageBuffer* image = create_image_buffer(width, height);
    if (!image) return NULL;
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Create a colorful gradient
            unsigned char r = (unsigned char)((x * 255) / width);
            unsigned char g = (unsigned char)((y * 255) / height);
            unsigned char b = (unsigned char)(((x + y) * 255) / (width + height));
            
            Pixel* pixel = get_pixel(image, x, y);
            if (pixel) {
                pixel->r = r;
                pixel->g = g;
                pixel->b = b;
                pixel->a = 255;
            }
        }
    }
    
    return image;
}

// Print image statistics
void print_image_stats(ImageBuffer* image, const char* name) {
    if (!image || !image->pixels) return;
    
    int r_min = 255, r_max = 0;
    int g_min = 255, g_max = 0;
    int b_min = 255, b_max = 0;
    
    for (size_t i = 0; i < image->pixel_count; ++i) {
        const Pixel* pixel = &image->pixels[i];
        
        r_min = pixel->r < r_min ? pixel->r : r_min;
        r_max = pixel->r > r_max ? pixel->r : r_max;
        g_min = pixel->g < g_min ? pixel->g : g_min;
        g_max = pixel->g > g_max ? pixel->g : g_max;
        b_min = pixel->b < b_min ? pixel->b : b_min;
        b_max = pixel->b > b_max ? pixel->b : b_max;
    }
    
    printf("[Image Stats] %s\n", name);
    printf("  Red:   [%d, %d]\n", r_min, r_max);
    printf("  Green: [%d, %d]\n", g_min, g_max);
    printf("  Blue:  [%d, %d]\n", b_min, b_max);
    printf("  Size:  %dx%d\n", image->width, image->height);
}

// =============================================================================
// Main Demo Function
// =============================================================================

int main() {
    printf("ISP Pipeline Demo (C Implementation)\n");
    printf("====================================\n");
    
    // Create test image
    printf("\nCreating test image...\n");
    ImageBuffer* inputImage = create_test_image(640, 480);
    ImageBuffer* outputImage = create_image_buffer(640, 480);
    
    if (!inputImage || !outputImage) {
        printf("Failed to create images!\n");
        return -1;
    }
    
    print_image_stats(inputImage, "Input Image");
    
    // Create ISP pipeline
    printf("\nCreating ISP pipeline...\n");
    ISPPipeline* pipeline = create_isp_pipeline("Camera ISP Pipeline");
    if (!pipeline) {
        printf("Failed to create pipeline!\n");
        free_image_buffer(inputImage);
        free_image_buffer(outputImage);
        return -1;
    }
    
    // Add processing modules in order
    printf("Adding processing modules...\n");
    add_module_to_pipeline(pipeline, create_black_level_module());
    add_module_to_pipeline(pipeline, create_white_balance_module());
    add_module_to_pipeline(pipeline, create_gamma_module());
    add_module_to_pipeline(pipeline, create_noise_reduction_module());
    
    // Configure some modules
    printf("\nConfiguring modules...\n");
    ISPModule* wb = get_module_by_name(pipeline, "WhiteBalance");
    if (wb && wb->configure) {
        wb->configure(wb, "red_gain", 1.3f);
        wb->configure(wb, "blue_gain", 1.0f);
    }
    
    ISPModule* gamma = get_module_by_name(pipeline, "GammaCorrection");
    if (gamma && gamma->configure) {
        gamma->configure(gamma, "gamma", 2.0f);
    }
    
    ISPModule* nr = get_module_by_name(pipeline, "NoiseReduction");
    if (nr && nr->configure) {
        nr->configure(nr, "strength", 0.4f);
    }
    
    // Process image through pipeline
    printf("\nProcessing image through ISP pipeline...\n");
    if (process_image_through_pipeline(pipeline, inputImage, outputImage)) {
        printf("ISP processing completed successfully!\n");
        print_image_stats(outputImage, "Output Image");
    } else {
        printf("ISP processing failed!\n");
        free_image_buffer(inputImage);
        free_image_buffer(outputImage);
        free_isp_pipeline(pipeline);
        return -1;
    }
    
    // Demonstrate module enable/disable
    printf("\nDemonstrating module control...\n");
    disable_module(pipeline, "NoiseReduction");
    
    ImageBuffer* outputImage2 = create_image_buffer(640, 480);
    if (outputImage2) {
        printf("\nProcessing with Noise Reduction disabled...\n");
        if (process_image_through_pipeline(pipeline, inputImage, outputImage2)) {
            printf("Processing completed!\n");
            print_image_stats(outputImage2, "Output Image (NR disabled)");
        }
        
        free_image_buffer(outputImage2);
    }
    
    // Re-enable modules
    enable_module(pipeline, "NoiseReduction");
    
    // Cleanup
    free_image_buffer(inputImage);
    free_image_buffer(outputImage);
    free_isp_pipeline(pipeline);
    
    printf("\nDemo completed!\n");
    return 0;
}
