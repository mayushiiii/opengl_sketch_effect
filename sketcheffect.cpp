#include "sketcheffect.h"

#include <vector>
#include <iostream>

#include "pfd/portable-file-dialogs.h"

using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

void GLAPIENTRY
MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
    //std::abort();
}



Tema2::Tema2()
{
    outputMode = 0;
    gpuProcessing = true;
    saveScreenToImage = false;
    window->SetSize(600, 600);
    framebuffer_object = 0;

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    // Load default texture fore imagine processing
    originalImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "pos_x.png"), nullptr, "image", true, true);
    processedImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "pos_x.png"), nullptr, "newImage", true, true);

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema2", "shaders");

    // Create a shader program for particle system
    {
        Shader* shader = new Shader("ImageProcessing");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentShader.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    CreateFramebuffer(originalImage->GetWidth(), originalImage->GetHeight());
}


void Tema2::FrameStart()
{
}

void Tema2::CreateFramebuffer(int width, int height)
{
    // TODO(student): In this method, use the attributes
    // 'framebuffer_object', 'color_texture' and 'depth_texture'
    // declared in lab3.h

    // TODO(student): Generate and bind the framebuffer
    glGenFramebuffers(1, &framebuffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
    // TODO(student): Generate, bind and initialize the color texture

    glGenTextures(1, &color_texture);
    glBindTexture(GL_TEXTURE_2D, color_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // TODO(student): Bind the color texture to the
    // framebuffer as a color attachment at position 0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);


    // TODO(student): Generate, bind and initialize the depth texture

    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // TODO(student): Bind the depth texture to the framebuffer as a depth attachment

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);


    // TODO(student): Set the color texture as the draw texture
    std::vector<GLenum> draw_textures;

    draw_textures.push_back(GL_COLOR_ATTACHMENT0 + color_texture);
    glDrawBuffers(draw_textures.size(), draw_textures.data());

    // TODO(student): Check the status of the framebuffer
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        return;
    }
}


void Tema2::Update(float deltaTimeSeconds)
{
    ClearScreen();

    auto shader = shaders["ImageProcessing"];
    shader->Use();

    if (saveScreenToImage)
    {
        window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
    }

    int flip_loc = shader->GetUniformLocation("flipVertical");
    glUniform1i(flip_loc, saveScreenToImage ? 0 : 1);

    int screenSize_loc = shader->GetUniformLocation("screenSize");
    glm::ivec2 resolution = window->GetResolution();
    glUniform2i(screenSize_loc, resolution.x, resolution.y);

    int outputMode_loc = shader->GetUniformLocation("outputMode");
    glUniform1i(outputMode_loc, outputMode);

    int locTexture = shader->GetUniformLocation("textureImage");
    glUniform1i(locTexture, 0);

    auto textureImage = (gpuProcessing == true) ? originalImage : processedImage;
    textureImage->BindToTextureUnit(GL_TEXTURE0);
    if (outputMode == 4)
    {
        // Blur orizontal
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
        glViewport(0, 0, resolution.x,resolution.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->Use();
        glUniform1i(shader->GetUniformLocation("outputMode"), 3); 

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureImage->GetTextureID());

        RenderMesh(meshes["quad"], shader, glm::mat4(1));

        // Blur vertical

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, resolution.x*1.75, resolution.y*1.75);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->Use();
        glUniform1i(shader->GetUniformLocation("outputMode"), 4); 
        glUniform1i(flip_loc, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, color_texture);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }

    if (outputMode >= 5 && outputMode <= 8) {


        // Blur
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
        glViewport(0, 0, resolution.x, resolution.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->Use();
        glUniform1i(shader->GetUniformLocation("outputMode"), 9);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureImage->GetTextureID());

        RenderMesh(meshes["quad"], shader, glm::mat4(1));

        // Hasurare

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, resolution.x * 1.75, resolution.y * 1.75);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->Use();
        glUniform1i(shader->GetUniformLocation("outputMode"), outputMode); 
        glUniform1i(flip_loc, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, color_texture);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }



	else
	{
        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }



    if (saveScreenToImage)
    {
        saveScreenToImage = false;

        GLenum format = GL_RGB;
        if (originalImage->GetNrChannels() == 4)
        {
            format = GL_RGBA;
        }

        glReadPixels(0, 0, originalImage->GetWidth(), originalImage->GetHeight(), format, GL_UNSIGNED_BYTE, processedImage->GetImageData());
        processedImage->UploadNewData(processedImage->GetImageData());
        SaveImage("shader_processing_" + std::to_string(outputMode));

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}


void Tema2::FrameEnd()
{
    DrawCoordinateSystem();
}


void Tema2::OnFileSelected(const std::string& fileName)
{
    if (fileName.size())
    {
        std::cout << fileName << endl;
        originalImage = TextureManager::LoadTexture(fileName, nullptr, "image", true, true);
        processedImage = TextureManager::LoadTexture(fileName, nullptr, "newImage", true, true);

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}


void Tema2::GrayScale()
{
    unsigned int channels = originalImage->GetNrChannels();
    unsigned char* data = originalImage->GetImageData();
    unsigned char* newData = processedImage->GetImageData();

    if (channels < 3)
        return;

    glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

    for (int i = 0; i < imageSize.y; i++)
    {
        for (int j = 0; j < imageSize.x; j++)
        {
            int offset = channels * (i * imageSize.x + j);

            // Reset save image data
            char value = static_cast<char>(data[offset + 0] * 0.2f + data[offset + 1] * 0.71f + data[offset + 2] * 0.07);
            memset(&newData[offset], value, 3);
        }
    }

    processedImage->UploadNewData(newData);
}


void Tema2::SaveImage(const std::string& fileName)
{
    cout << "Saving image! ";
    processedImage->SaveToFile((fileName + ".png").c_str());
    cout << "[Done]" << endl;
}


void Tema2::OpenDialog()
{
    std::vector<std::string> filters =
    {
        "Image Files", "*.png *.jpg *.jpeg *.bmp",
        "All Files", "*"
    };

    auto selection = pfd::open_file("Select a file", ".", filters).result();
    if (!selection.empty())
    {
        std::cout << "User selected file " << selection[0] << "\n";
        OnFileSelected(selection[0]);
    }
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}


void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
    }

    if (key == GLFW_KEY_E)
    {
        gpuProcessing = !gpuProcessing;
        if (gpuProcessing == false)
        {
            outputMode = 0;
        }
        cout << "Processing on GPU: " << (gpuProcessing ? "true" : "false") << endl;
    }

    if (key - GLFW_KEY_0 >= 0 && key <= GLFW_KEY_9)
    {
        outputMode = key - GLFW_KEY_0;


        if (gpuProcessing == false)
        {
            outputMode = 0;
            GrayScale();
        }
    }

    if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
    {
        if (!gpuProcessing)
        {
            SaveImage("processCPU_" + std::to_string(outputMode));
        }
        else {
            saveScreenToImage = true;
        }
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema2::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
